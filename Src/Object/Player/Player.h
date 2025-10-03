#pragma once
#include <string>
#include <DxLib.h>

class Player
{
public:
    Player(int id)
        : ctrlId_(id), pos_(VGet(0, 0, 0)), radius_(1.0f), height_(2.0f), velY_(0.0f), gravity_(-0.01f), isOnGround_(false)
    {
    }

    virtual ~Player() = default;

    // 基本処理（純粋仮想ではなく基底に実装）
    virtual void Init() {}
    virtual void Update() { UpdateMove(); }
    virtual void Draw() {}
    virtual void Attack() {}

	// ID取得
	int GetID() const { return ctrlId_; }

    // 名前取得
    virtual std::string GetCharacterName() const { return "Player"; }

    // 座標・コライダー情報の統一
    VECTOR GetPos() const { return pos_; }
    void SetPos(const VECTOR& pos) { pos_ = pos; }
    float GetRadius() const { return radius_; }
    float GetHeight() const { return height_; }

protected:


    void UpdateMove()
    {
        // 横移動（デフォルト）
        if (CheckHitKey(KEY_INPUT_UP))    pos_.z += speed_;
        if (CheckHitKey(KEY_INPUT_DOWN))  pos_.z -= speed_;
        if (CheckHitKey(KEY_INPUT_LEFT))  pos_.x -= speed_;
        if (CheckHitKey(KEY_INPUT_RIGHT)) pos_.x += speed_;

        // Y軸（ジャンプ＋重力）
        if (CheckHitKey(KEY_INPUT_1) && isOnGround_)
        {
            velY_ = 0.3f;
            isOnGround_ = false;
        }

        velY_ += gravity_;
        pos_.y += velY_;

        // 底面補正（暫定）
        if (pos_.y - height_ / 2.0f < 0.0f)
        {
            pos_.y = height_ / 2.0f;
            velY_ = 0.0f;
            isOnGround_ = true;
        }
    }

protected:
    int ctrlId_;
    VECTOR pos_;
    float radius_;
    float height_;
    float velY_;
    const float gravity_;
    bool isOnGround_;
    float speed_ = 10.2f;
};
