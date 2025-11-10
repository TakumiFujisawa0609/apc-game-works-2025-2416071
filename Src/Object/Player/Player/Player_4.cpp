#include "Player_4.h"
#include <DxLib.h>
#include "../../../Utility/AsoUtility.h"
#include "../Control/InputController.h"

void Player_4::Init()
{
	// 親クラスの初期化
	Player::Init();

	// モデルの読み込み
	modelId_ = MV1LoadModel("Data/Model/Player/testModel.mv1");

	param_.speed = 6.0f;
	param_.jumpPower = 7.0f;
}

void Player_4::Update()
{
	// 親クラスの更新処理
	Player::Update();

	// 入力で攻撃処理
	if (controller_->IsAttackTrigger())
	{
		Shot();
	}
}

void Player_4::Release()
{
	MV1DeleteModel(modelId_);
}