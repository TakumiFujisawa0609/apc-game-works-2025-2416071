#include "Stage.h"
#include <DxLib.h>
#include "../Player/Player.h"

Stage* Stage::instance_ = nullptr;



Stage::Stage(void)
{
}

Stage::~Stage(void)
{
}

void Stage::CreateInstance(void)
{
    if (instance_ == nullptr)
    {
        instance_ = new Stage();
    }
    instance_->Init();
}

Stage& Stage::GetInstance(void)
{
    if (instance_ == nullptr)
    {
        Stage::CreateInstance();
    }
    return *instance_;
}

void Stage::Init(void)
{
    // モデルの読み込み
    modelId_ = MV1LoadModel("Data/Model/Stage/Stage.mv1");

    if (modelId_ == -1)
    {
        DxLib::DxLib_End();
        exit(-1);
    }

    pos_ = DEFAULT_POS;
    angle_ = { 0.0f, 0.0f, 0.0f };
    scale_ = { 1.0f, 1.0f, 1.0f };

    // 円柱コライダー設定（仮の半径/高さ）
    cylinder_.center = pos_;
    cylinder_.radius = 250.0f;
    cylinder_.yMin = pos_.y;            // 床
    cylinder_.yMax = pos_.y + 1000.0f;  // 高さ100
}

void Stage::Update(void)
{
  
	IsInsideStage(VECTOR{ 0,0,0 });
}

void Stage::Draw(void)
{
	// モデルの描画
	MV1SetPosition(modelId_, pos_);
	MV1SetRotationXYZ(modelId_, angle_);
	MV1SetScale(modelId_, scale_);
	MV1DrawModel(modelId_);
}

void Stage::Release(void)
{
	MV1DeleteModel(modelId_);
}

void Stage::SetParam(int playerNum)
{

}

bool Stage::IsInsideStage(const VECTOR& pos) const
{
	// プレイヤーの位置がステージ内にあるか判定
	// プレイヤーの位置を取得
	/*VECTOR PlayerPos = player_->GetPos();
	PlayerPos = pos;*/

    // 仮でx,zが±500以内ならステージ内とする
    if (pos.x < 500.0f && pos.x > -500.0f &&
        pos.z < 500.0f && pos.z > -500.0f)
    {
        return true;
    }
}



