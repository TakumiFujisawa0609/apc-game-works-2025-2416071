#include "Player_1.h"

constexpr float GROUND_Y = 1.0f;

void Player_1::Init(void)
{
    modelId_ = MV1LoadModel("Data/Model/Player/testModel.mv1");
    pos_ = VGet(0.0f, 100.0f, 0.0f);
    radius_ = 1.0f;
    height_ = 2.0f;
    speed_ = 10.2f;
    velY_ = 0.0f;
    isOnGround_ = false;
}

void Player_1::Update(void)
{
    UpdateMove();
}

void Player_1::Draw(void)
{
    // モデルを描画
    MV1SetPosition(modelId_, pos_);
    MV1SetScale(modelId_, VGet(1.0f, 1.0f, 1.0f));
    MV1DrawModel(modelId_);

    // プレイヤーの識別表示
    DrawFormatString(600, 20, GetColor(255, 255, 255), "Player_1");

    // プレイヤーの位置をデバッグ表示
    DrawFormatString(0, 40, GetColor(255, 255, 255), "Player_1 Pos:(%.1f, %.1f, %.1f)", pos_.x, pos_.y, pos_.z);
    // プレイヤーの大きさ
    DrawFormatString(0, 60, GetColor(255, 255, 255), "Player_1 Size: Radius(%.1f) Height(%.1f)", radius_, height_);


}

void Player_1::Release(void)
{
    MV1DeleteModel(modelId_);
}

void Player_1::UpdateMove(void)
{
    // 横移動
    if (CheckHitKey(KEY_INPUT_UP))    pos_.z += speed_;
    if (CheckHitKey(KEY_INPUT_DOWN))  pos_.z -= speed_;
    if (CheckHitKey(KEY_INPUT_LEFT))  pos_.x -= speed_;
    if (CheckHitKey(KEY_INPUT_RIGHT)) pos_.x += speed_;

    // ジャンプ処理
    if (CheckHitKey(KEY_INPUT_1) && isOnGround_)
    {
        velY_ = 0.3f; // ジャンプ初速度
        isOnGround_ = false;
    }

    // 重力
    velY_ += gravity_;
    pos_.y += velY_;

    // --- 地面との当たり判定 ---
    // プレイヤーの底面が地面より下に行ったら補正
    float playerBottom = pos_.y - height_ / 2.0f;
    if (playerBottom < GROUND_Y)
    {
        pos_.y = GROUND_Y + height_ / 2.0f;
        velY_ = 0.0f;
        isOnGround_ = true;
    }
    else
    {
        isOnGround_ = false;
    }
}

void Player_1::Attack(void)
{
    // 攻撃処理をここに実装
    std::cout << "Player_1 Attack!" << std::endl;
}