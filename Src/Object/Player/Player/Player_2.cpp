#include "Player_2.h"
#include <DxLib.h>
#include "../../../Manager/InputManager.h"
#include "../../../Utility/AsoUtility.h"
#include "../Control/InputController.h"


void Player_2::Init(void)
{
	// 親クラスの初期化
	Player::Init();
	// モデルの読み込み
	modelId_ = MV1LoadModel("Data/Model/Player/Capsule_15.mv1");
	pos_ = { 60.0f, 0.0f, 0.0f };
	angle_ = { 0.0f, AsoUtility::Deg2RadF(0.0f), 1.0f }; // 初期向きはX+方向
	param_.speed = 6.0f;
	param_.jumpPower = 7.0f;
}

void Player_2::Release(void)
{
	// モデルの解放
	MV1DeleteModel(modelId_);
	modelId_ = -1;
}

void Player_2::Update()
{
	// 親クラスの更新処理
	Player::Update();

	// 入力で攻撃処理
	if (controller_->IsAttackTrigger())
	{
		Shot();
	}
}