#pragma once
#include "InputController.h"
#include <DxLib.h>                               // VECTOR 型
#include "../../../Manager/InputManager.h"        // InputManager

class Controller : public InputController
{
public:
    // コンストラクタ: KeyConfig を受け取る
    Controller(const KeyConfig& config, InputManager::JOYPAD_NO padNo);

    // 入力ベクトル取得
    VECTOR GetMoveInputVector() const override;

    // ジャンプトリガ取得
    bool IsJumpTrigger() const override;

    // 攻撃トリガ取得
    bool IsAttackTrigger() const override;

    // 追加: ダッシュのため(チャージ)ボタンのホールド
    bool IsDashChargeHeld() const override;

private:
    // このコントローラ用のキー設定
    KeyConfig config_;

    // このプレイヤーに割り当てたパッド
    InputManager::JOYPAD_NO padNo_;
};