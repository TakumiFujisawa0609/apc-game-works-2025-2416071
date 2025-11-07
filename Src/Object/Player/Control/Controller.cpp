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

	// ゲームパッドが接続していない場合のキー処理
	if(GetJoypadNum() == 0)
	{
		if (ins.IsNew(config_.up)) worldInputVec.z += 10.0f;
		if (ins.IsNew(config_.down)) worldInputVec.z -= 10.0f;
		if (ins.IsNew(config_.left)) worldInputVec.x -= 10.0f;
		if (ins.IsNew(config_.right)) worldInputVec.x += 10.0f;
	}
	else
	{

		// 自分のプレイヤー番号に応じたパッド入力を取得
		InputManager::JOYPAD_IN_STATE padState = ins.GetJPadInputState(padNo_);
		worldInputVec = ins.GetDirXZAKey(padState.AKeyLX, padState.AKeyLY);
	}
	
	return worldInputVec;
}

bool Controller::IsJumpTrigger() const
{
	InputManager& ins = InputManager::GetInstance();

    // キーボードジャンプ
    if (ins.IsTrgDown(config_.jump))
        return true;

    // Padジャンプ（例: R_TRIGGERボタンをジャンプに割り当て）
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

	// キーボード攻撃
	if (ins.IsTrgDown(config_.attack)) // config_.attack キーを使用
		return true;

	// Pad攻撃（例: DOWNボタンを攻撃に割り当て）
	if (GetJoypadNum() > 0) {
		// ここでは例として DOWN ボタンを使用するが、
		// R_TRIGGERや他のボタンも選択肢として考えられる
		if (ins.IsPadBtnTrgDown(padNo_, InputManager::JOYPAD_BTN::DOWN)) {
			return true;
		}
	}

	return false;
}
