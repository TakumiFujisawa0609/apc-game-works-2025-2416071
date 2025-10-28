#include <DxLib.h>
#include "../Control/InputController.h"
#include "../../Stage/Stage.h"
#include "../../../Utility/MatrixUtility.h"
#include "../../../Utility/AsoUtility.h"
#include "../../../Manager/InputManager.h"
#include "../../AttackObj/Common/AttackObj.h"
#include "PlayerManager.h"
#include "Player.h"

// 静的メンバ変数の定義
int Player::nextDeathOrder_ = 1;

// コンストラクタ
Player::Player(int id, const PlayerParam& param, std::unique_ptr<InputController> controller)
	:id_(id), param_(param), controller_(std::move(controller))
{

}

Player::~Player()
{
}

void Player::Init()
{
	// モデルの読み込みは派生クラスで実施

	// 変数の初期化　リセット可能な状態をここで設定
	pos_ = { 0.0f,0.0f,0.0f };
	moveVec_ = AsoUtility::VECTOR_ZERO;
	param_.speed = 15.0f;
	angle_ = { 0.0f,AsoUtility::Deg2RadF(180.0f),1.0f };
	modelId_ = -1;
	inputVecNor_ = AsoUtility::VECTOR_ZERO;
}

void Player::Update()
{
	// 生存していない場合は更新しない
	if (!isAlive_)
	{
		// プレイヤーを落としていく
		pos_.y -= GRAVITY;
		return;
	}

	// ステージのインスタンス取得
	Stage& stage = Stage::GetInstance();

	// ステージの外にいるかどうかを判定
	if (!isFalling_ && !stage.IsPlayerOnStage(pos_))
	{
		// 落下開始
		isFalling_ = true;
		// 水平速度を初期化
		moveVec_ = AsoUtility::VECTOR_ZERO;
	}

	// 落下中の処理
	if (isFalling_)
	{
		// 重力で落下
		moveVec_.y -= GRAVITY;

		// モデルを反映
		MV1SetPosition(modelId_, pos_);

		// 一定の高さ以下に来たら死亡扱いにする
		if (pos_.y < stage.GetPos().y - 5000.0f)
		{
			// 死亡処理
			Die();
		}
		// それ以外の処理は行わない
		return;

	}

	// 通常の移動処理
	Move();

	// 地面の高さに合わせて位置を補正
	// レイではなく、カプセルで判定する
	ApplyStageGround(stage);

	// ステージの傾きを取得
	VECTOR tilt = stage.GetAngle();
	float tiltX = tilt.x;
	float tiltZ = tilt.z;

	// 重力加速度(坂滑り)
	const float gravityAccel = 0.3f;
	VECTOR slopeAccel = VGet(
		sinf(tiltZ) * gravityAccel,
		0.0f,
		sinf(tiltX) * gravityAccel
	);

	// 傾きに基づいて速度を計算
	moveVec_ = VAdd(moveVec_, slopeAccel);

	// 摩擦
	moveVec_ = VScale(moveVec_, param_.friction);

	// 最大速度制限
	float len = VSize(moveVec_);
	if (len > param_.maxSpeed)
	{
		moveVec_ = VScale(moveVec_, param_.maxSpeed / len);
	}

	// 攻撃処理
	Attack();

	// モデルの位置更新
	pos_ = VAdd(pos_, moveVec_);

	// 落下死確認
	if (pos_.y < stage.GetPos().y - 5000.0f)
	{
		// 死亡処理
		Die();
	}
	
}

void Player::Draw()
{
	MV1SetPosition(modelId_, pos_);

	// 向きの設定
	// float rotY = atan2f(inputVecNor_.x, inputVecNor_.z);
	// VECTOR rot = VGet(0.0f, rotY + AsoUtility::Deg2RadF(180.0f), 0.0f);	

	// XZ平面での角度を計算
	float rotY = atan2f(-inputVecNor_.x, -inputVecNor_.z);
	VECTOR rot = VGet(0.0f, rotY, 0.0f);

	MV1SetRotationXYZ(modelId_, rot);

	// モデルの描画
	if(pos_.y >= -5000.0f)
		MV1DrawModel(modelId_);

	// デバッグ
	// デバッグ表示
	DrawSphere3D(pos_, 0.5f, 16, GetColor(255, 0, 0), GetColor(255, 0, 0), TRUE);
	DrawLine3D(pos_, VAdd(pos_, moveVec_), GetColor(0, 255, 0));
	DrawFormatString(0, 0 + id_ * 20, GetColor(255, 255, 255), "Player %d Pos: (%.2f, %.2f, %.2f)", id_ + 1, pos_.x, pos_.y, pos_.z);

	// ★ カプセルデバッグ描画
	VECTOR p1 = pos_;
	VECTOR p2 = pos_;
	float halfHeight = CAPSULE_HEIGHT / 2.0f;
	p1.y += halfHeight;
	p2.y -= halfHeight;
	DrawCapsule3D(p1, p2, collisionRadius_, 16, GetColor(0, 0, 255), GetColor(0, 0, 255), FALSE);


	if (isAlive_) {
		// 生存時の表示
		DrawFormatString(850, 600 + id_ * 20, GetColor(0, 255, 0), "Player %d: Alive", id_ + 1);
	}
	else {
		// 死亡時の表示
		DrawFormatString(850, 600 + id_ * 20, GetColor(255, 0, 0), "Player %d: Dead", id_ + 1);
	}


	// プレイヤーのパラメータを表示
	DrawFormatString(500, 540 + id_ * 20, GetColor(255, 0, 255), "PlayerID: %d Weight: %.2f Speed: %.2f JumpPower: %.2f", id_, param_.weight, param_.speed, param_.jumpPower);
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
		// 地面にいる時のみジャンプ可能にする判定は、ApplyStageGroundのmoveVec_.y = 0.0f;に依存する
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
    const float capsuleRadius = collisionRadius_;
    const float capsuleHalfHeight = CAPSULE_HEIGHT / 2.0f;

	float groundY = 0.0f;
    // 地面の高さ取得
    groundY = stage.GetGroundHeight(pos_, capsuleRadius, capsuleHalfHeight);

    // --- 異常値対策 ---
    if (!std::isfinite(groundY) || groundY < -10000.0f || groundY > 10000.0f)
    {
        // 無効値なら重力のみ適用
        isFalling_ = true;
        moveVec_.y -= GRAVITY;
        return;
    }

    float feetY = pos_.y - capsuleHalfHeight;
	
    if (feetY <= groundY)
    {
        // 地面に接地している or めり込んでいる
        pos_.y = groundY + capsuleHalfHeight;
        moveVec_.y = 0.0f;
        isFalling_ = false;
    }
    else
    {
        // 空中 → 落下
        isFalling_ = true;
        moveVec_.y -= GRAVITY;
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