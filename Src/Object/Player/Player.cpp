#include "Player.h"
#include "../../Utility/MatrixUtility.h"	// Lerp用
#include "../../Utility/AsoUtility.h"
#include "../../Manager/InputManager.h"
#include "../Stage/Stage.h"
#include <DxLib.h>

Player::Player(int id, float weight,int inputId)
	:id_(id), weight_(weight), pos_(AsoUtility::VECTOR_ZERO), moveVec_(AsoUtility::VECTOR_ZERO), speed_(0.0f), modelId_(-1)
{
}

Player::~Player()
{
}

void Player::Init()
{
	// モデルの読み込み
	// 派生クラスで実装

	// 変数は初期化
	moveVec_ = { 0.0f,0.0f,0.0f };
	speed_ = 15.0f;
	worldInputVec = { 0.0f,0.0f,0.0f };
	inputId_ = 0;

}

void Player::Update()
{
	// 処理自体は派生クラスで実装
	Move();

	// ステージの地面に合わせてY座標を補正
	Stage& stage = Stage::GetInstance();
	ApplyStageGround(stage);
}

void Player::Draw()
{
	MV1SetPosition(modelId_, pos_);
	MV1DrawModel(modelId_);

	// デバッグ用に座標に球を描画
	DrawSphere3D(pos_, 0.5f, 16, GetColor(255, 0, 0), GetColor(255, 0, 0), TRUE);

	// 座標に線を描画
	DrawLine3D(pos_, VAdd(pos_, moveVec_), GetColor(0, 255, 0));

	// プレイヤー座標を表示
	DrawFormatString(0, 0, GetColor(255, 255, 255), "Player %d Pos: (%.2f, %.2f, %.2f)", id_ + 1, pos_.x, pos_.y, pos_.z);

	// プレイヤー_2の座標を表示
	if (id_ == 1) {
		DrawFormatString(0, 20, GetColor(255, 255, 255), "Player %d Pos: (%.2f, %.2f, %.2f)", id_ + 1, pos_.x, pos_.y, pos_.z);
	}

	// inputId_ を表示
	DrawFormatString(0, 40, GetColor(255, 255, 255), "Player %d InputId: %d", id_ + 1, inputId_);
}


void Player::Move()
{
	// ステージのインスタンス取得
	Stage& stage = Stage::GetInstance();

	// ステージの傾きを取得
	VECTOR stageAngle = stage.GetAngle();

	// 入力管理インスタンス取得
	InputManager& ins = InputManager::GetInstance();

	// 回転行列の準備
	MATRIX rotX = MGetRotX(stageAngle.x);
	MATRIX rotZ = MGetRotZ(stageAngle.z);
	MATRIX stageRotationMatrix = MMult(rotZ, rotX);

	// ステージの回転 (ローカル -> ワールド)
	MATRIX invStageRotationMatrix = MTranspose(stageRotationMatrix);


	// worldInputVec をローカル変数として宣言し、毎回初期化する！
	VECTOR worldInputVec = { 0.0f, 0.0f, 0.0f };

	// inputId_ に応じた入力処理
	if (inputId_ == 0)
	{
		// P1のキー操作 (WASD)
		// ※ここでは IsPress() の方が自然だが、IsNew()を維持して進める
		if (ins.IsNew(KEY_INPUT_W)) worldInputVec.z += 10.0f;
		if (ins.IsNew(KEY_INPUT_S)) worldInputVec.z -= 10.0f;
		if (ins.IsNew(KEY_INPUT_A)) worldInputVec.x -= 10.0f;
		if (ins.IsNew(KEY_INPUT_D)) worldInputVec.x += 10.0f;
	}
	else if (inputId_ == 1) 
	{
		// P2のキー操作 (矢印キー)
		if (ins.IsNew(KEY_INPUT_UP)) worldInputVec.z += 10.0f;
		if (ins.IsNew(KEY_INPUT_DOWN)) worldInputVec.z -= 10.0f;
		if (ins.IsNew(KEY_INPUT_LEFT)) worldInputVec.x -= 10.0f;
		if (ins.IsNew(KEY_INPUT_RIGHT)) worldInputVec.x += 10.0f;
	}
	// P3, P4 の入力も else if でここに追加

	// ワールドの重力ベクトル（下向き）
	VECTOR worldGravity = { 0.0f, -GRAVITY * SLIDE_FACTOR, 0.0f };

	// 重力ベクトルを逆行列
	VECTOR slideAccel = VTransform(worldGravity, invStageRotationMatrix);

	// 垂直方向(Y)の力は無視
	slideAccel.y = 0.0f;

	//操作による加速 A_Input の計算
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

	// ジャンプ入力も inputId_ で切り替え！
	if ((inputId_ == 0 && ins.IsTrgDown(KEY_INPUT_SPACE)) ||
		(inputId_ == 1 && ins.IsTrgDown(KEY_INPUT_RETURN)))
	{
		moveVec_.y += 5.0f; // ジャンプ力を調整
	}
}

void Player::ApplyStageGround(const Stage& stage)
{
	// 1. ステージの地面の高さを取得
	float groundY = stage.GetGroundHeight(pos_);

	pos_.y = MatrixUtility::Lerp(pos_.y, groundY, 0.1f);
}

