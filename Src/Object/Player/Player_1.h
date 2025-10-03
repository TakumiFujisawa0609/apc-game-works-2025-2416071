#pragma once
#include "Player.h"
#include <DxLib.h>
#include <iostream>


class Player_1 : public Player
{
public:
    Player_1(int id, float weight = 1.0f)
        : Player(id, weight) {} // 基底コンストラクタ呼び出し

    void Init() override;
    void Update() override;
    void Draw() override;
    void Release();
    void Attack() override;
    std::string GetCharacterName() const override { return "Player_1"; }

private:
    int modelId_;
};


