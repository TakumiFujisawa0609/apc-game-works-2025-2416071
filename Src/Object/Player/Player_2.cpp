#include "Player_2.h"
#include <DxLib.h>

void Player_2::Init(void)
{
	// 親クラスの初期化
	Player::Init();
	// モデルの読み込み
	modelId_ = MV1LoadModel("Data/Model/Player/testModel.mv1");
	pos_ = { -50.0f, 0.0f, -30.0f };
	speed_ = 5.0f;
}

void Player_2::Update(void)
{
	// 親クラスの更新
Player::Update();

	// モデルの描画
	MV1SetPosition(modelId_, pos_);
	MV1SetRotationXYZ(modelId_, pos_);
}

void Player_2::Draw(void)
{
	
	MV1DrawModel(modelId_);
}

void Player_2::Release(void)
{
}
