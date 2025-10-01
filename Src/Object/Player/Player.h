#pragma once
#include <string>
#include <DxLib.h>

class Player
{


public:

    Player(int id) : ctrlId_(id), pos_(VGet(0, 0, 0)), radius_(1), height_(2), speed_(0) {}
    virtual ~Player() = default;


    virtual void Init();
    virtual void Update();
    virtual void Draw();
    virtual void Attack();
    virtual void Release();

    // 座標操作
    VECTOR GetPos() const { return pos_; }
    void SetPos(const VECTOR& v) { pos_ = v; }
    float GetRadius() const { return radius_; }
    float GetHeight() const { return height_; }

    // キャラクター名取得
    virtual std::string GetCharacterName() const = 0;

	// プレイヤーID取得
	int GetID() const { return ctrlId_; }

protected:
    VECTOR pos_;
    float radius_;
    float height_;
    float speed_;
    int modelId_;
    int ctrlId_; // コントローラーID

};
