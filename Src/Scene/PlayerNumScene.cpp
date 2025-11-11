#include "../Manager/InputManager.h"
#include "../Manager/SceneManager.h"
#include "../Object/UIInput.h"
#include "PlayerNumScene.h"

PlayerNumScene::PlayerNumScene(void)
{
}

PlayerNumScene::~PlayerNumScene(void)
{
}

void PlayerNumScene::Init(void)
{
	selectNum_ = SELECT::SELECT_1P;
}

void PlayerNumScene::Update(void)
{
	// キー + PAD1 の入力を統合
	auto pi = UIInput::GetPlayerNumInput();

	// 左 / 右 で選択移動
	if (pi.left)
	{
		selectNum_--;
		if (selectNum_ < SELECT::SELECT_1P)
		{
			selectNum_ = SELECT::SELECT_MAX - 1;
		}
	}
	else if (pi.right)
	{
		selectNum_++;
		if (selectNum_ >= SELECT::SELECT_MAX)
		{
			selectNum_ = SELECT::SELECT_1P;
		}
	}

	// 決定
	if (pi.decide)
	{
		SceneManager::GetInstance().SetPlayerNum(GetSelectNum());
		SceneManager::GetInstance().ChangeScene(SceneManager::SCENE_ID::CHARASELECT);
		return;
	}

	// 戻る（タイトルへ）
	if (pi.back)
	{
		SceneManager::GetInstance().ChangeScene(SceneManager::SCENE_ID::TITLE);
		return;
	}
}

void PlayerNumScene::Draw(void)
{
	// タイトル
	(100, 40, GetColor(255, 255, 255), -1, "プレイヤー人数選択画面");

	auto pi = UIInput::GetPlayerNumInput();

	// PAD1 未接続なら注意（ただしキーボードは使用可能）
	if (!pi.pad1Connected)
	{
		DrawFormatString2(100, 90, GetColor(255, 180, 120), -1, "注意: PAD1 未接続。キーボードで操作できます。");
	}

	// 現在人数
	DrawFormatString(100, 120, GetColor(255, 255, 255), "プレイヤー人数: %d", GetSelectNum());

	// 選択肢表示
	for (int i = 0; i < SELECT::SELECT_MAX; i++)
	{
		const bool isCur = (i == selectNum_);
		unsigned int col = isCur ? GetColor(255, 255, 0) : GetColor(255, 255, 255);
		if (isCur)
		{
			DrawFormatString2(100, 170 + i * 40, col, -1, "> %d Player", i + 1);
		}
		else
		{
			DrawFormatString2(100, 170 + i * 40, col, -1, "  %d Player", i + 1);
		}
	}

	// 操作ガイド
	DrawFormatString2(500, 170, GetColor(180, 180, 255), -1, "PAD1: X=左 / B=右 / A=決定 / Y=戻る");
	DrawFormatString2(500, 210, GetColor(180, 180, 255), -1, "KEY : ←/A/↑/W=左, →/D/↓/S=右, Enter/Space=決定, B=戻る");
}

void PlayerNumScene::Release(void)
{

}
