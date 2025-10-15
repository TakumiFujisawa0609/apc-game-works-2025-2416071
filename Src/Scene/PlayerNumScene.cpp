#include "../Manager/InputManager.h"
#include "../Manager/SceneManager.h"
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
	// 人数選択
	InputManager& ins = InputManager::GetInstance();
	if (ins.IsTrgDown(KEY_INPUT_UP))
	{
		selectNum_--;
		if (selectNum_ < SELECT::SELECT_1P)
		{
			selectNum_ = SELECT::SELECT_MAX - 1;
		}
	}
	else if (ins.IsTrgDown(KEY_INPUT_DOWN))
	{
		selectNum_++;
		if (selectNum_ >= SELECT::SELECT_MAX)
		{
			selectNum_ = SELECT::SELECT_1P;
		}
	}

	// 決定
	if (ins.IsTrgDown(KEY_INPUT_A))
	{
		// 次のシーンへ
		SceneManager::GetInstance().ChangeScene(SceneManager::SCENE_ID::CHARASELECT);
		SceneManager::GetInstance().SetPlayerNum(selectNum_);
	}
}

void PlayerNumScene::Draw(void)
{
	// タイトル表示
	DrawFormatString2(100, 40, GetColor(255, 255, 255), -1, "プレイヤー人数選択画面");

	DrawFormatString(100, 90, GetColor(255, 255, 255), "プレイヤー人数 %n",(int)(selectNum_));

	// 点滅
	int time = GetNowCount() / 500;
	if (time % 2 == 0)
	{
		DrawFormatString2(300, 200, GetColor(255, 0, 0), -1, "試遊版はプレイヤー人数を3人以上登録可能ですが、\n 2人プレイまで対応しています");
		// 警告　1人目は対応していないので、赤字で表示
		DrawFormatString2(300, 240, GetColor(255, 0, 0), -1, "警告: 1人プレイはデバッグ用です。\n	選択しないでください");

	}


	

	// 選択人数表示
	for (int i = 0; i < SELECT::SELECT_MAX; i++)
	{
		if (i == selectNum_)
		{
			// 選択中
			DrawFormatString2(100, 140 + i * 40, GetColor(255, 255, 0), -1, "> %d Player", i + 1);
		}
		else
		{
			// 選択していない
			DrawFormatString2(100, 140 + i * 40, GetColor(255, 255, 255), -1, "  %d Player", i + 1);
		}
	}

	// 背景色の水色は絶対に見せてはならないので、背景画像は描画必須
}

void PlayerNumScene::Release(void)
{

}
