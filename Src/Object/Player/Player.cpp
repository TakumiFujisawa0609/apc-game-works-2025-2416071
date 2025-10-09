#include "Player.h"
#include "../../Utility/MatrixUtility.h"	// Lerp用
#include "../../Utility/AsoUtility.h"
#include "../../Manager/InputManager.h"
#include "../Stage/Stage.h"
#include <DxLib.h>

Player::Player(int id, float weight)
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
	DrawFormatString(0, 20 * id_, GetColor(255, 255, 255), "Player %d Pos: (%.2f, %.2f, %.2f)", id_, pos_.x, pos_.y, pos_.z);
}



void Player::Move()
{
	
}

void Player::ApplyStageGround(const Stage& stage)
{
	// 1. ステージの地面の高さを取得
	float groundY = stage.GetGroundHeight(pos_);

	pos_.y = MatrixUtility::Lerp(pos_.y, groundY, 0.1f);
}

