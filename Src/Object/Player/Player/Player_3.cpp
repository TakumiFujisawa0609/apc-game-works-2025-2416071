#include "Player_3.h"
#include <DxLib.h>
#include "../../../Utility/AsoUtility.h"
#include "../Control/InputController.h"


void Player_3::Init()
{
	// 親クラスの初期化
	Player::Init();

	// モデルの読み込み
	modelId_ = MV1LoadModel("Data/Model/Player/testModel.mv1");

	param_.speed = 6.0f;
	param_.jumpPower = 7.0f;

}

void Player_3::Release()
{
	MV1DeleteModel(modelId_);
}
