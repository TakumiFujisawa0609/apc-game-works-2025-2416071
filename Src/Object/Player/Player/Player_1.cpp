#include "Player_1.h"
#include <DxLib.h>
#include "../../../Utility/MatrixUtility.h"	// Lerp用
#include "../../../Manager/InputManager.h"
#include "../../../Utility/AsoUtility.h"
#include "../Control/InputController.h"
#include "../Common/PlayerManager.h"


void Player_1::Init()
{
	// 親クラスの初期化
	Player::Init();

	// モデルの読み込み
	modelId_ = MV1LoadModel("Data/Model/Player/testModel.mv1");

	pos_ = { -60.0f, 0.0f, 0.0f };

	angle_ = { 0.0f, AsoUtility::Deg2RadF(180.0f), AsoUtility::Deg2RadF(180.0f) }; // 初期向きはZ+方向

	param_.jumpPower = 7.0f;

}

void Player_1::Update()
{
	// 親クラスの更新処理
	Player::Update();

	// 入力で攻撃処理
	if (controller_->IsAttackTrigger())
	{
		Shot();
	}
}

void Player_1::Release()
{
	// モデルの解放
	MV1DeleteModel(modelId_);
	modelId_ = -1;
}
