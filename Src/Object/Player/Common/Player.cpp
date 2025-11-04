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
	// 落下時死亡判定チェック
	if (pos_.y < -1000.0f)
	{
		Die();
	}

	Stage& stage = Stage::GetInstance();

	// ステージ外にいるかどうか
	VECTOR stageCenter = stage.GetPos();			// ステージの中心位置
	float stageRadius = stage.GetCollider().radius; // ステージの半径

	// プレイヤーの座標からステージの中心までの距離を計算
	float dx = pos_.x - stageCenter.x;
	float dz = pos_.z - stageCenter.z;
	float distXZ = sqrtf(dx * dx + dz * dz);

	// ステージ外に出たら落下する
	if (!isFalling_ && distXZ > stageRadius)
	{
		isFalling_ = true;
		// 水平速度をリセット
		moveVec_.x = 0.0f;
	}

	// 落下中の処理
	if (isFalling_)
	{
		// 落下速度を加算
		moveVec_.y -= GRAVITY_ACCEL; // GRAVITY_ACCELは別途定義
		// 位置更新
		pos_ = VAdd(pos_, moveVec_);
		// ステージ内に戻ったら落下状態解除
		if (distXZ <= stageRadius)
		{
			isFalling_ = false;
			// Y位置をステージの地面に合わせる
			ApplyStageGround(stage);
			// Y速度をリセット
			moveVec_.y = 0.0f;
		}
		return; // 落下中は他の処理を行わない
	}

	// 1. 入力による移動処理と坂滑りの計算
	Move();

	// 2. 重力
	moveVec_.y -= GRAVITY_ACCEL;		// 常に重力を加算する

	// 3. 地面の高さに合わせて位置補正
	ApplyStageGround(stage);

	// 4. 摩擦処理 (Moveから移動)
	const float PLAYER_FRICTION = 0.85f; // Move()から定数を移動
	moveVec_ = VScale(moveVec_, PLAYER_FRICTION);

	// 5. 最大速度制限 (Moveから移動)
	float len = VSize(moveVec_);
	if (len > param_.maxSpeed)
	{
		moveVec_ = VScale(moveVec_, param_.maxSpeed / len);
	}

	// 6. 攻撃
	Attack();

	// 7. 位置更新
	pos_ = VAdd(pos_, moveVec_);

	//モデルに反映
	MV1SetPosition(modelId_, pos_);
}

void Player::Move()
{// ステージのインスタンスを取得
	Stage& stage = Stage::GetInstance();
	// 角度を取得
	VECTOR stageAngle = stage.GetAngle();

	// 回転行列
	MATRIX rotX = MGetRotX(stageAngle.x);
	MATRIX rotZ = MGetRotZ(stageAngle.z);
	MATRIX stageRotMat = MMult(rotZ, rotX);
	MATRIX invStageRotationMat = MTranspose(stageRotMat); // 逆行列は転置行列

	// 入力ベクトルの取得
	VECTOR worldInputVec = controller_->GetMoveInputVector();
	// 入力ベクトルを正規化　0除算対策
	float inputLen = VSize(worldInputVec);
	if (worldInputVec.x != 0.0f || worldInputVec.z != 0.0f)
	{
		inputVecNor_ = VNorm(worldInputVec); // 入力方向の正規化
	}
	else
	{
		// セーフティ
		inputVecNor_ = { 0.0f, 0.0f, 1.0f }; // デフォルトの向き
	}

	// 上り坂の判定に使用するベクトル（XZ平面での傾き方向）
	// ステージ傾きを基に坂の方向ベクトルを算出
	VECTOR slopeDir = VGet(sinf(stageAngle.z), 0.0f, sinf(stageAngle.x));
	// slopeFactor: 入力ベクトルと坂の向きの内積
	float slopeFactor = VDot(inputVecNor_, slopeDir);

	// 基本の速度と最大速度を初期化
	float moveSpeed = param_.speed;
	float maxSpeed = param_.maxSpeed;

	// 上り坂の時(減速)
	if (slopeFactor > 0.01f)
	{
		const float maxUphillSlow = 0.5f; // 最大減速率
		float factor = slopeFactor * 2.0f;
		if (factor > 1.0f) factor = 1.0f;

		float speedMultiplier = 1.0f - (maxUphillSlow * factor);

		// 基本の速度と最大速度を減速分に合わせて調整
		moveSpeed *= speedMultiplier;
		maxSpeed *= speedMultiplier;			// maxSpeedの調整はUpdateの制限で利用
	}
	// 下り坂の時(加速)
	else if (slopeFactor < -0.01f)
	{
		const float maxDownhillBoost = 0.4f;	// 最大加速率
		float factor = fabsf(slopeFactor) * 2.0f;
		if (factor > 1.0f) factor = 1.0f;

		float speedMultiplier = 1.0f + maxDownhillBoost * factor;

		// 基本速度と最大速度を加速分に合わせて調整
		moveSpeed *= speedMultiplier;
		maxSpeed *= speedMultiplier;			// maxSpeedの調整はUpdateの制限で利用
	}

	// 1. 入力による加速度計算
	VECTOR inputAcc = AsoUtility::VECTOR_ZERO;
	if (worldInputVec.x != 0.0f || worldInputVec.z != 0.0f)
	{
		// 移動方向をステージの傾き補正逆行列で変換
		VECTOR moveDir = VTransform(inputVecNor_, invStageRotationMat);
		// Y成分は無視
		moveDir.y = 0.0f;

		// 調節後のmovespeedを使って加速度を計算
		inputAcc = VScale(moveDir, moveSpeed * INPUT_ACCEL_FACTOR); // INPUT_ACCEL_FACTORは別途定義
	}

	// 2. 下り坂・重力による滑り (X軸/Z軸両方向の滑り)
	const float GRAVITY = 9.8f;
	const float SLIDE_FACTOR = 0.5f;

	// 速度に加速度を加算
	VECTOR worldGravity = VGet(0.0f, -GRAVITY * SLIDE_FACTOR, 0.0f); // 滑り係数を重力に適用
	VECTOR slideAcc = VTransform(worldGravity, invStageRotationMat);
	slideAcc.y = 0.0f; // Y成分は無視

	// 移動ベクトル更新 (Updateの摩擦/速度制限前にMoveの加速度を加算)
	moveVec_ = VAdd(moveVec_, inputAcc);
	moveVec_ = VAdd(moveVec_, slideAcc);

	// 3. ジャンプ処理
	if (controller_->IsJumpTrigger())
	{
		// 地面にいる場合のみジャンプ可能
		float groundY = stage.GetGroundHeight(pos_);
		float requiredY = groundY + (MODEL_CENTER_TO_FEET / 2.0f); // MODEL_CENTER_TO_FEETは別途定義
		if (pos_.y <= requiredY + 1.0f) // 少しの誤差を許容
		{
			moveVec_.y = param_.jumpPower;
		}
	}

	// 4. 向き更新
	if (worldInputVec.x != 0.0f || worldInputVec.z != 0.0f)
	{
		// 移動後の速度ベクトルから向きを計算
		VECTOR flatMoveDir = { moveVec_.x,0.0f,moveVec_.z };
		if (VSize(flatMoveDir) > 0.0f) {
			angle_ = VNorm(flatMoveDir);
		}
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
