#include "Player.h"
#include "../../Utility/MatrixUtility.h"
#include "../../Utility/AsoUtility.h"
#include "../../Manager/InputManager.h"
#include "../Stage/Stage.h"
#include "Control/InputController.h"
#include <DxLib.h>


// 静的メンバ
int Player::nextDeathOrder_ = 1;

// コンストラクタ
Player::Player(int id, float weight, std::unique_ptr<InputController> controller)
// 必須メンバとcontrollerの所有権を初期化子リストで設定
	: id_(id),
	weight_(weight),
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
	speed_ = 15.0f;
	modelId_ = -1; // Initでロードしない場合は-1で初期化
}

void Player::Update()
{
	// 生存状態が偽なら更新しない
	if (!isAlive_)
	{
		return;
	}

	Move();

	// ステージの地面に合わせてY座標を補正
	ApplyStageGround(Stage::GetInstance());

	// 死亡判定: Y座標が-100以下なら死亡
	if (pos_.y < -100.0f)
	{
		Die();
	}
}

void Player::Draw()
{
	MV1SetPosition(modelId_, pos_);
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

	// 1Pには赤色、2Pには青色の球体を表示
	VECTOR spherPos = { pos_.x + 50.f ,pos_.y + 100.f,pos_.z };
	DrawSphere3D(spherPos, 20.0f, 10, (id_ == 0) ? GetColor(255, 0, 0) : GetColor(0, 0, 255), (id_ == 0) ? GetColor(255, 0, 0) : GetColor(0, 0, 255), TRUE);


}

void Player::Move()
{
	Stage& stage = Stage::GetInstance();

	// ステージの傾きを取得
	VECTOR stageAngle = stage.GetAngle();

	// 回転行列の準備
	MATRIX rotX = MGetRotX(stageAngle.x);
	MATRIX rotZ = MGetRotZ(stageAngle.z);
	MATRIX stageRotationMatrix = MMult(rotZ, rotX);
	MATRIX invStageRotationMatrix = MTranspose(stageRotationMatrix);

	// InputControllerから移動入力を取得
	VECTOR worldInputVec = controller_->GetMoveInputVector();

	// ワールドの重力ベクトル
	VECTOR worldGravity = { 0.0f, -GRAVITY * SLIDE_FACTOR, 0.0f };

	// 重力ベクトルを逆行列で変換
	VECTOR slideAccel = VTransform(worldGravity, invStageRotationMatrix);
	slideAccel.y = 0.0f; // 垂直方向の力は無視

	// 操作による加速 A_Input の計算
	VECTOR inputAccel = AsoUtility::VECTOR_ZERO;
	if (worldInputVec.x != 0.0f || worldInputVec.z != 0.0f) {
		VECTOR actualMoveDir = VTransform(VNorm(worldInputVec), invStageRotationMatrix);
		actualMoveDir.y = 0.0f;
		inputAccel = VScale(actualMoveDir, speed_ * INPUT_ACCEL_FACTOR);
	}

	// 移動ベクトルに加速度を加算
	moveVec_ = VAdd(moveVec_, slideAccel);
	moveVec_ = VAdd(moveVec_, inputAccel);

	// 既存の速度に摩擦（減衰）を適用
	moveVec_ = VScale(moveVec_, PLAYER_FRICTION);

	// 移動ベクトルの長さが最大速度を超えていたら、最大速度に制限
	if (AsoUtility::MagnitudeF(moveVec_) > MAX_SPEED) {
		moveVec_ = VScale(VNorm(moveVec_), MAX_SPEED);
	}

	// 座標に移動ベクトルを加算
	pos_ = VAdd(pos_, moveVec_);
	
	// ジャンプ入力
	if (controller_->IsJumpTrigger())
	{
		moveVec_.y += 5.0f; // ジャンプ力を加算
	}
}

void Player::ApplyStageGround(const Stage& stage)
{
	// ステージの地面の高さを取得し、Lerpで補正
	float groundY = stage.GetGroundHeight(pos_);
	pos_.y = MatrixUtility::Lerp(pos_.y, groundY, 0.1f);
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
