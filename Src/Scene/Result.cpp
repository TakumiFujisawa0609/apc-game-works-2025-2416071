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
	// PlayerManagerから勝者IDを取得
	playerWinID_ = PlayerManager::GetInstance().GetWinnerID();

	// PlayerManagerから各プレイヤーのスコアを取得
	//auto ranks = PlayerManager::GetInstance().GetPlayerRanks();

	// 入力取得
	InputManager& ins = InputManager::GetInstance();
	// リザルト画面から次のシーンへ移行
	// SceneManager取得



	if (ins.IsNew(KEY_INPUT_RETURN) || ins.IsPadBtnNew(InputManager::JOYPAD_NO::PAD1, InputManager::JOYPAD_BTN::RIGHT))
	{
		SceneManager::GetInstance().ChangeScene(SceneManager::SCENE_ID::TITLE);
	}
}

void Result::Draw(void)
{
	// ここに描画処理を追加していく

	// 勝者表示
	if (playerWinID_ != -1) {
		DrawFormatString(100, 100, GetColor(255, 255, 0), "勝者：Player %d ", playerWinID_ + 1);
	}

	// PAD1の右ボタンまたはEnterキーでタイトルへ戻る 操作促し文字
	DrawString(100, 200, "PAD1 / Bボタンでタイトルへ", GetColor(255, 255, 255));
}

void Result::Release(void)
{
}
