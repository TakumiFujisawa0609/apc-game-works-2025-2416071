#pragma once
#include <DxLib.h>

// キー設定
struct KeyConfig
{
    int up;
    int down;
    int left;
    int right;
    int jump;
    int attack;
    int dashCharge; 
};

class InputController
{
public:
    virtual ~InputController() = default;

    // XZ 平面の移動入力
    virtual VECTOR GetMoveInputVector() const = 0;

    // ジャンプ
    virtual bool IsJumpTrigger() const = 0;

    // 攻撃
    virtual bool IsAttackTrigger() const = 0;

    // 追加: ダッシュの「ため」(チャージ) ホールド
    virtual bool IsDashChargeHeld() const { return false; }
};