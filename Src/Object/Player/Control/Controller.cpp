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

    // ゲームパッド接続時は左スティックの連続ベクトル
    if (GetJoypadNum() > 0)
    {
        InputManager::JOYPAD_IN_STATE padState = ins.GetJPadInputState(padNo_);
        // AKeyLX, AKeyLY を XZ 平面ベクトルへ変換（環境既存のヘルパを使用）
        worldInputVec = ins.GetDirXZAKey(padState.AKeyLX, padState.AKeyLY);
    }
    else
    {
        // 未接続時はキーボード（パルス的にベクトルを与える）
        if (ins.IsNew(config_.up))    worldInputVec.z += 10.0f;
        if (ins.IsNew(config_.down))  worldInputVec.z -= 10.0f;
        if (ins.IsNew(config_.left))  worldInputVec.x -= 10.0f;
        if (ins.IsNew(config_.right)) worldInputVec.x += 10.0f;
    }

    return worldInputVec;
}

bool Controller::IsJumpTrigger() const
{
    InputManager& ins = InputManager::GetInstance();

    // キーボード・ジャンプ
    if (ins.IsTrgDown(config_.jump))
        return true;

    // パッド・ジャンプ（例: R_TRIGGER）
    if (GetJoypadNum() > 0) {
        if (ins.IsPadBtnTrgDown(padNo_, InputManager::JOYPAD_BTN::R_TRIGGER)) {
            return true;
        }
    }

    return false;
}

bool Controller::IsAttackTrigger() const
{
    InputManager& ins = InputManager::GetInstance();

    // キーボード・攻撃
    if (ins.IsTrgDown(config_.attack))
        return true;

    // パッド・攻撃（例: 十字キーDOWN）
    if (GetJoypadNum() > 0) {
        if (ins.IsPadBtnTrgDown(padNo_, InputManager::JOYPAD_BTN::DOWN)) {
            return true;
        }
    }

    return false;
}

bool Controller::IsDashChargeHeld() const
{
    InputManager& ins = InputManager::GetInstance();

    if (GetJoypadNum() > 0)
    {
    
        if (ins.IsPadBtnTrgDown(padNo_, InputManager::JOYPAD_BTN::L_TRIGGER)) {
            return true;
        }

        InputManager::JOYPAD_IN_STATE padState = ins.GetJPadInputState(padNo_);
        const float lt = padState.AKeyLY;
        if (lt > 0.5f) return true;
    }

    // キーボード（共用）: Shift は「ため」
    if (CheckHitKey(KEY_INPUT_LSHIFT) || CheckHitKey(KEY_INPUT_RSHIFT)) {
        return true;
    }

    return false;
}