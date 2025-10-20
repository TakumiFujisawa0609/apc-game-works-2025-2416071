#include "Player_2.h"

void Player_2::Init(void)
{
	// 親クラスの初期化
	Player::Init();
	// モデルの読み込み
	modelId_ = MV1LoadModel("Data/Model/Player/Capsule_15.mv1");
	pos_ = { 60.0f, 0.0f, 0.0f };
	param_.speed = 6.0f;
	param_.jumpPower = 7.0f;
}

void Player_2::Release(void)
{
	MV1DeleteModel(modelId_);
}
