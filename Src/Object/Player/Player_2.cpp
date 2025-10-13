#include "Player_2.h"

void Player_2::Init(void)
{
	// 親クラスの初期化
	Player::Init();
	// モデルの読み込み
	modelId_ = MV1LoadModel("Data/Model/Player/testModel.mv1");
	pos_ = { 0.0f, 0.0f, 0.0f };
	speed_ = 5.0f;
}

void Player_2::Release(void)
{
	MV1DeleteModel(modelId_);
}
