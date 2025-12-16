#pragma once
#include "InputController.h"
#include <DxLib.h>
#include "../../../Manager/InputManager.h"

class Controller : public InputController
{
public:
    Controller(const KeyConfig& config, InputManager::JOYPAD_NO padNo);

    VECTOR GetMoveInputVector() const override;
    bool IsJumpTrigger() const override;
    bool IsAttackTrigger() const override;

    // LT or キーボードの dashCharge キー
    bool IsDashChargeHeld() const override;

private:
    KeyConfig config_;
    InputManager::JOYPAD_NO padNo_;
};