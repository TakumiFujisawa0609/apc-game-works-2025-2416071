#include "Controller.h"
#include "../../../Utility/AsoUtility.h"
#include "../../../Scene/PlayerNumScene.h"

Controller::Controller(const KeyConfig& config, InputManager::JOYPAD_NO padNo)
    : config_(config), padNo_(padNo)
{
}

VECTOR Controller::GetMoveInputVector() const
{
    InputManager& ins = InputManager::GetInstance();
    VECTOR worldInputVec = { 0.0f, 0.0f, 0.0f };

    if (GetJoypadNum() == 0)
    {
        if (ins.IsNew(config_.up))    worldInputVec.z += 10.0f;
        if (ins.IsNew(config_.down))  worldInputVec.z -= 10.0f;
        if (ins.IsNew(config_.left))  worldInputVec.x -= 10.0f;
        if (ins.IsNew(config_.right)) worldInputVec.x += 10.0f;
    }
    else
    {
        InputManager::JOYPAD_IN_STATE padState = ins.GetJPadInputState(padNo_);
        worldInputVec = ins.GetDirXZAKey(padState.AKeyLX, padState.AKeyLY);
    }

    return worldInputVec;
}

bool Controller::IsJumpTrigger() const
{
    InputManager& ins = InputManager::GetInstance();

    if (ins.IsTrgDown(config_.jump)) return true;

    if (GetJoypadNum() > 0) {
        if (ins.IsPadBtnTrgDown(padNo_, InputManager::JOYPAD_BTN::R_TRIGGER)) return true;
    }
    return false;
}

bool Controller::IsAttackTrigger() const
{
    InputManager& ins = InputManager::GetInstance();

    if (ins.IsTrgDown(config_.attack)) return true;

    if (GetJoypadNum() > 0) {
        if (ins.IsPadBtnTrgDown(padNo_, InputManager::JOYPAD_BTN::DOWN)) return true;
    }
    return false;
}

bool Controller::IsDashChargeHeld() const
{
    InputManager& ins = InputManager::GetInstance();

    if (GetJoypadNum() > 0)
    {
        // 推奨: トリガ押しっぱなしAPI
        if (ins.IsPadBtnNew(padNo_, InputManager::JOYPAD_BTN::L_TRIGGER) || // 代替（押下検知）
            ins.IsPadBtnTrgDown(padNo_, InputManager::JOYPAD_BTN::L_TRIGGER)) {
            // 押下イベントのみの場合は、必要なら「押しっぱなし状態」を自前で保持してください
        }
        // 左トリガーのアナログ値での判定が必要なら、InputManagerにLT値を追加して参照してください
    }

    // キーボード: プレイヤー別の dashCharge キーを使用
    return CheckHitKey(config_.dashCharge) != 0;
}