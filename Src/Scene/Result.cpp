#include "Result.h"
#include "../Object/Player/Common/PlayerManager.h"
#include "../Manager/InputManager.h"
#include "../Manager/SceneManager.h"

Result::Result(void)
{
}

Result::~Result(void)
{
}

void Result::Init(void)
{
	// 初期化
	playerWinID_ = -1;
	for (int i = 0; i < 4; ++i) {
		playerScores_[i] = 0;
	}
}

void Result::Update(void)
{
	// Winner（-1 の場合はゲームオーバー表示用）
	playerWinID_ = PlayerManager::GetInstance().GetWinnerID();

	// 入力
	InputManager& ins = InputManager::GetInstance();

	// タイトルへ
	if (ins.IsNew(KEY_INPUT_RETURN) || ins.IsPadBtnNew(InputManager::JOYPAD_NO::PAD1, InputManager::JOYPAD_BTN::RIGHT))
	{
		SceneManager::GetInstance().ChangeScene(SceneManager::SCENE_ID::TITLE);
	}
}

void Result::Draw(void)
{
	if (playerWinID_ == -1) {
		// 1人プレイのゲームオーバー
		DrawString(100, 100, "GAME OVER", GetColor(255, 120, 120));
	}
	else {
		// 勝者表示
		DrawFormatString(100, 100, GetColor(255, 255, 0), "Winner: Player %d", playerWinID_ + 1);
	}

	DrawString(100, 200, "PAD1 / Bボタン または Enter でタイトルへ", GetColor(255, 255, 255));
}

void Result::Release(void)
{
}