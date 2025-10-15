#pragma once
#include "InputController.h"
#include <DxLib.h>								// VECTOR型のために必要
#include "../../../Manager/InputManager.h"		// InputManagerを使うために必要

class Controller : public InputController
{
public:
	// コンストラクタ: KeyConfigを受け取る
	Controller(const KeyConfig& config, InputManager::JOYPAD_NO padNo);

	// 入力ベクトルを取得
	VECTOR GetMoveInputVector() const override;

	// ジャンプトリガーを取得
	bool IsJumpTrigger() const override;

private:

	// このコントローラーが使うキー設定を保持
	KeyConfig config_;

	// パッドの接続数
	InputManager::JOYPAD_NO padNo_;
};