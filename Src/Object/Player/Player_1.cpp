#include "Player_1.h"
#include <DxLib.h>
#include "../../Utility/MatrixUtility.h"	// Lerp用
#include "../../Manager/InputManager.h"
#include "../../Utility/AsoUtility.h"


void Player_1::Init()
{
	// 親クラスの初期化
	Player::Init();

	// モデルの読み込み
	modelId_ = MV1LoadModel("Data/Model/Player/testModel.mv1");

	pos_ = { -60.0f, 0.0f, 0.0f };

	param_.jumpPower = 7.0f;

}

void Player_1::Release()
{
	MV1DeleteModel(modelId_);
}
