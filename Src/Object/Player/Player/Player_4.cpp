#include "Player_4.h"
#include <DxLib.h>
#include "../../../Utility/AsoUtility.h"
#include "../Control/InputController.h"

void Player_4::Init()
{
	// 親クラスの初期化
	Player::Init();

	// モデルの読み込み
	modelId_ = MV1LoadModel("Data/Model/Player/Body_AD_01.mv1");

	//param_.speed = 6.0f;
	param_.jumpPower = 700.0f;
}

void Player_4::Update()
{
	// 親クラスの更新処理
	Player::Update();

	if (pos_.y == -1200.0f)
	{
		pos_.y = -1200.0f;
		return;
	}

	// 入力で攻撃処理
	if (controller_->IsAttackTrigger())
	{
		Shot();
	}
}

void Player_4::Release()
{
	// モデルの解放
	MV1DeleteModel(modelId_);
	modelId_ = -1;
}