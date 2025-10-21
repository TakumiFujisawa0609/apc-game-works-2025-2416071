#include <DxLib.h>
#include "../Control/InputController.h"
#include "../../Stage/Stage.h"
#include "../../../Utility/MatrixUtility.h"
#include "../../../Utility/AsoUtility.h"
#include "../../../Manager/InputManager.h"
#include "../../AttackObj/Common/AttackObj.h"
#include "PlayerManager.h"
#include "Player.h"


// 静的メンバ
int Player::nextDeathOrder_ = 1;

// コンストラクタ
Player::Player(int id, const PlayerParam& param, std::unique_ptr<InputController> controller)
	: id_(id),
	param_(param),
	controller_(std::move(controller))
{
}

Player::~Player()
{
}

void Player::Init()
{
	// モデルの読み込みは派生クラスで実装

	// 変数の初期化 (リセット可能な状態をInitで設定)
	pos_ = { 0.0f, 0.0f, 0.0f }; // 初期座標を設定
	moveVec_ = { 0.0f,0.0f,0.0f };
	param_.speed = 15.0f;
	angle_ = { 0.0f, AsoUtility::Deg2RadF(180.0f), 1.0f}; // 初期向きはZ+方向}
	modelId_ = -1; // Initでロードしない場合は-1で初期化

	inputVecNor_ = {0.0f, 0.0f, 1.0f };
}

void Player::Update()
{
	// 生存していない場合は処理しない
	if (!isAlive_ )
	{
		pos_.y -= 5.0f; // 死亡後の落下アニメーション
		return;
	}


	Stage& stage = Stage::GetInstance();

	// --- ステージ外チェック ---
	VECTOR stageCenter = stage.GetPos();          // ステージ中心座標
	float stageRadius = stage.GetCollider().radius; // ステージ半径

	float dx = pos_.x - stageCenter.x;
	float dz = pos_.z - stageCenter.z;
	float distanceXZ = sqrtf(dx * dx + dz * dz);

	// --- ステージ外に出た瞬間に落下モードへ ---
	if (!isFalling_ && distanceXZ > stageRadius)
	{
		isFalling_ = true;
		moveVec_ = { 0.0f, 0.0f, 0.0f }; // 水平速度リセット
	}

	// --- 落下中処理 ---
	if (isFalling_)
	{
		// 重力で落下
		moveVec_.y -= 0.98f;  // 落下加速度
		pos_ = VAdd(pos_, moveVec_);

		// モデル反映
		MV1SetPosition(modelId_, pos_);

		// 一定高さまで落ちたら死亡
		if (pos_.y < -100.0f)
		{
			Die();
		}

		return; // 通常処理はスキップ
	}

	// --- 通常移動処理 ---
	Move();

	// 地面の高さに合わせて補正
	ApplyStageGround(stage);

	// ステージ傾き取得
	VECTOR tilt = stage.GetAngle();
	float tiltX = tilt.x;
	float tiltZ = tilt.z;

	// 重力加速度（坂滑り）
	const float gravityAccel = 0.3f;
	VECTOR slopeAccel = VGet(
		sinf(tiltZ) * gravityAccel,  // Z軸傾きでX方向に加速
		0.0f,
		-sinf(tiltX) * gravityAccel  // X軸傾きでZ方向に加速
	);

	// 傾きに基づく速度を加算
	moveVec_ = VAdd(moveVec_, slopeAccel);

	// 摩擦
	moveVec_ = VScale(moveVec_, param_.friction);

	// 最大速度制限
	float len = VSize(moveVec_);
	if (len > param_.maxSpeed)
	{
		moveVec_ = VScale(moveVec_, param_.maxSpeed / len);
	}

	// 攻撃（任意の実装がある場合）
	Attack();

	// 位置更新
	pos_ = VAdd(pos_, moveVec_);

	// モデル反映
	MV1SetPosition(modelId_, pos_);

	// --- 落下死亡チェック ---
	if (pos_.y < -1000.0f)
	{
		Die();
	}
}

void Player::Draw()
{
	MV1SetPosition(modelId_, pos_);

	// 向きの設定
	//float rotY = atan2f(-angle_.x, -angle_.z); // XZ平面での角度を計算
	//VECTOR rot = { 0.0f, rotY, 0.0f };

	float rotY = atan2f(-inputVecNor_.x, -inputVecNor_.z); // XZ平面での角度を計算
	VECTOR rot = { 0.0f, rotY, 0.0f };



	MV1SetRotationXYZ(modelId_, rot);

	if(pos_.y  >= -1000.0f)
	MV1DrawModel(modelId_);

	// デバッグ表示
	DrawSphere3D(pos_, 0.5f, 16, GetColor(255, 0, 0), GetColor(255, 0, 0), TRUE);
	DrawLine3D(pos_, VAdd(pos_, moveVec_), GetColor(0, 255, 0));
	DrawFormatString(0, 0 + id_ * 20, GetColor(255, 255, 255), "Player %d Pos: (%.2f, %.2f, %.2f)", id_ + 1, pos_.x, pos_.y, pos_.z);

	if (isAlive_) {
		// 生存時の表示
		DrawFormatString(850, 600 + id_ * 20, GetColor(0, 255, 0), "Player %d: Alive", id_ + 1);
	}
	else {
		// 死亡時の表示
		DrawFormatString(850, 600 + id_ * 20, GetColor(255, 0, 0), "Player %d: Dead", id_ + 1);
	}

	// 1Pには赤色、2Pには青色、3Pには緑色、4Pには黄色の球体を表示
	DrawSphere3D(pos_, 50.0f, 16, GetColor(255 * (id_ == 0), 255 * (id_ == 2), 255 * (id_ == 1)), GetColor(255 * (id_ == 0), 255 * (id_ == 2), 255 * (id_ == 1)), TRUE);





	// プレイヤーのパラメータを表示
	DrawFormatString(500, 540 + id_ * 20, GetColor(255, 0, 255), "PlayerID: %d Weight: %.2f Speed: %.2f JumpPower: %.2f",id_, param_.weight, param_.speed, param_.jumpPower);


}


void Player::Move()
{
	Stage& stage = Stage::GetInstance();
	VECTOR stageAngle = stage.GetAngle();

	// 回転行列（ステージ傾き補正用）
	MATRIX rotX = MGetRotX(stageAngle.x);
	MATRIX rotZ = MGetRotZ(stageAngle.z);
	MATRIX stageRotationMatrix = MMult(rotZ, rotX);
	MATRIX invStageRotationMatrix = MTranspose(stageRotationMatrix);

	// 入力ベクトル取得
	VECTOR worldInputVec = controller_->GetMoveInputVector();
	if (worldInputVec.x != 0.0f || worldInputVec.z != 0.0f) {
		inputVecNor_ = VNorm(worldInputVec); // 入力方向の正規化
	}

	// 上り坂判定に使用するベクトル（XZ平面でのステージの傾き方向）
	VECTOR slopeDir = VGet(std::sin(stageAngle.x), 0.0f, std::sin(stageAngle.z));
	// slopeFactor: >0 なら入力が上り坂方向、<0 なら下り坂方向
	float slopeFactor = VDot(inputVecNor_, slopeDir);

	// 基本速度と最大速度を初期化
	float moveSpeed = param_.speed;
	float currentMaxSpeed = param_.maxSpeed; // 初期値 15.0f

	// 上り坂減速・最大速度制限
	if (slopeFactor > 0.01f) {
		const float maxUphillSlow = 0.7f; // 最大減速率
		float factor = slopeFactor * 2.0f; // 傾斜が緩やかな場合は 0 に近くなる
		if (factor > 1.0f) factor = 1.0f;

		float speedMultiplier = 1.0f - maxUphillSlow * factor;

		// 基本速度と最大速度を減速分に合わせて調整
		moveSpeed *= speedMultiplier;
		currentMaxSpeed *= speedMultiplier;
	}
	// 下り坂加速・最大速度解放
	else if (slopeFactor < -0.01f) {
		const float maxDownhillBoost = 0.4f; // 最大加速率
		float factor = fabsf(slopeFactor) * 2.0f; // 傾斜が緩やかな場合は 0 に近くなる
		if (factor > 1.0f) factor = 1.0f;

		float speedMultiplier = 1.0f + maxDownhillBoost * factor;

		// 基本速度と最大速度を加速分に合わせて調整
		moveSpeed *= speedMultiplier;
		currentMaxSpeed *= speedMultiplier;
	}

	// 入力による加速度
	VECTOR inputAccel = AsoUtility::VECTOR_ZERO;
	if (worldInputVec.x != 0.0f || worldInputVec.z != 0.0f) {
		// 移動方向をステージの傾き補正逆行列で変換
		VECTOR moveDir = VTransform(inputVecNor_, invStageRotationMatrix);
		moveDir.y = 0.0f; // Y成分を無視

		// 調整後の moveSpeed を使用して加速度を計算
		inputAccel = VScale(moveDir, moveSpeed * INPUT_ACCEL_FACTOR);
	}

	// 下り坂・重力による滑り
	// worldGravityを傾き補正逆行列で変換し、ステージに沿った滑りベクトルを算出
	VECTOR worldGravity = { 0.0f, -GRAVITY * SLIDE_FACTOR, 0.0f };
	VECTOR slideAccel = VTransform(worldGravity, invStageRotationMatrix);
	slideAccel.y = 0.0f; // 水平方向のみ適用

	// 移動ベクトル更新
	moveVec_ = VAdd(moveVec_, inputAccel);
	moveVec_ = VAdd(moveVec_, slideAccel);

	// 摩擦
	moveVec_ = VScale(moveVec_, PLAYER_FRICTION);

	// 最大速度制限（動的に計算した currentMaxSpeed を使用）
	float speedLen = AsoUtility::MagnitudeF(moveVec_);
	if (speedLen > currentMaxSpeed) {
		moveVec_ = VScale(VNorm(moveVec_), currentMaxSpeed);
	}

	// 位置更新
	pos_ = VAdd(pos_, moveVec_);

	// ジャンプ
	if (controller_->IsJumpTrigger()) {
		moveVec_.y += param_.jumpPower;
	}

	// 向き更新（XZ平面）
	if (moveVec_.x != 0.0f || moveVec_.z != 0.0f) {
		VECTOR flatMoveVec = { moveVec_.x, 0.0f, moveVec_.z };
		angle_ = VNorm(flatMoveVec);
	}
}

void Player::ApplyStageGround(const Stage& stage)
{
	// ステージの地面の高さを取得
	float groundY = stage.GetGroundHeight(pos_);

	// 地面が存在しない場合（非常に低い値が返ってきた場合）は補正しない
	if (groundY < -500.0f) {
		// 地面がないので補正せず、そのまま落下させる
		return;
	}

	// 地面よりも下にいると判断するY座標のしきい値
	float requiredY = groundY + (MODEL_CENTER_TO_FEET / 2.0f);

	// プレイヤーが地面よりもめり込んでいたら強制的にPlayerの位置を補正
	if (pos_.y < requiredY) {
		// Y座標を強制的に地面へ合わせる
		pos_.y = requiredY;

		// Y方向の速度をリセット
		moveVec_.y = 0.0f;
	}
}


void Player::Die()
{
	// 既に死亡している場合は処理しない
	if (!isAlive_) return;

	// 死亡順序を設定し、次の順序に進める
	isAlive_ = false;

	// 死亡順序を設定
	deathOrder_ = nextDeathOrder_++;
}

void Player::Attack()
{
	// Fキー攻撃
	if (attackCooldown_ > 0)
	{
		attackCooldown_--;
	}

	// InputMnagerからキー入力取得
	InputManager& ins = InputManager::GetInstance();

	// Fキー入力とクールダウンチェック
	if (attackCooldown_ == 0 && ins.IsNew(KEY_INPUT_F))
	{
		attackCooldown_ = 30;

		VECTOR startPos = VAdd(pos_, VScale(inputVecNor_, 50.0f));

		float attackSpeed = 40.0f;
		AttackObj* newAttack = new BulletAttack(id_, startPos, inputVecNor_, attackSpeed);

		// 4. PlayerManagerに登録
		PlayerManager::GetInstance().AddAttackObject(newAttack);
	}
}

void Player::Release()
{
	if (modelId_ != -1)
	{
		MV1DeleteModel(modelId_);
		modelId_ = -1;
	}
}
