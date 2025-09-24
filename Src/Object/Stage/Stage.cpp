#include "Stage.h"

Stage::Stage(void)
{
}

Stage::~Stage(void)
{
}

void Stage::Init(void)
{
    // ÉÇÉfÉãÇÃì«Ç›çûÇ›
    modelId_ = MV1LoadModel("Data/Model/Stage/Stage.mv1");

    if (modelId_ == -1)
    {
        DxLib::DxLib_End();
        exit(-1);
    }

    pos_ = { 0.0f, -700.0f, 0.0f };
    angle_ = { 0.0f, 0.0f, 0.0f };
    scale_ = { 1.0f, 1.0f, 1.0f };
}

void Stage::Update(void)
{
  

}

void Stage::Draw(void)
{
	// ÉÇÉfÉãÇÃï`âÊ
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
