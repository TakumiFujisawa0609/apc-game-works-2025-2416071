#include "Player_1.h"

void Player_1::Init()
{
    modelId_ = MV1LoadModel("Data/Model/Player/testModel.mv1");
    pos_ = VGet(0.0f, 100.0f, 0.0f);
    radius_ = 1.0f;
    height_ = 2.0f;
    velY_ = 0.0f;
    isOnGround_ = false;
}

void Player_1::Update()
{
    UpdateMove(); // 基底の座標・重力処理を利用
}

void Player_1::Draw()
{
    MV1SetPosition(modelId_, pos_);
    MV1SetScale(modelId_, VGet(1.0f, 1.0f, 1.0f));
    MV1DrawModel(modelId_);

    DrawFormatString(0, 40, GetColor(255, 255, 255), "Player_1 Pos:(%.1f, %.1f, %.1f)", pos_.x, pos_.y, pos_.z);
}

void Player_1::Release()
{
    MV1DeleteModel(modelId_);
}

void Player_1::Attack()
{
    std::cout << "Player_1 Attack!" << std::endl;
}
