#include "../Manager/InputManager.h"
#include "../Manager/SceneManager.h"
#include "../Object/UIInput.h"
#include "../Application.h"
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

	// 背景画像読み込み
	bgImg_ = LoadGraph("Data/Image/Background.png");

	// 人数選択画像読み込み
	selectImg_[SELECT::SELECT_1P] = LoadGraph("Data/Image/1.png");
	selectImg_[SELECT::SELECT_2P] = LoadGraph("Data/Image/2.png");
	selectImg_[SELECT::SELECT_3P] = LoadGraph("Data/Image/3.png");
	selectImg_[SELECT::SELECT_4P] = LoadGraph("Data/Image/4.png");

	// 人数選択してね画像読み込み
	selectPromptImg_ = LoadGraph("Data/Image/PlayerNum.png");
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

	// 背景描画
	DrawRotaGraph(Application::SCREEN_SIZE_X / 2, Application::SCREEN_SIZE_Y / 2, 0.75, 0.0, bgImg_, TRUE);

	// 操作ガイド画像表示
	DrawRotaGraph(Application::SCREEN_SIZE_X / 2, Application::SCREEN_SIZE_Y / 2 - 200, 0.25, 0.0, selectPromptImg_, TRUE);

	// タイトル
	(100, 40, GetColor(255, 255, 255), -1, "プレイヤー人数選択画面");

	auto pi = UIInput::GetPlayerNumInput();

	// PAD1 未接続なら注意（ただしキーボードは使用可能）
	if (!pi.pad1Connected)
	{
		//DrawFormatString2(100, 90, GetColor(255, 180, 120), -1, "注意: PAD1 未接続。キーボードで操作できます。");
	}

	// 現在人数
	//DrawFormatString(100, 120, GetColor(255, 255, 255), "プレイヤー人数: %d", GetSelectNum());

	// 選択肢表示
	for (int i = 0; i < SELECT::SELECT_MAX; i++)
	{
		// ここに選択中の人数画像を表示する
		int color = (i == selectNum_) ? GetColor(255, 255, 0) : GetColor(255, 255, 255);
		//DrawFormatString2(100, 160 + i * 30, color, -1, "%d 人", i + 1);
	}

	switch (selectNum_)
	{
	case PlayerNumScene::SELECT_1P:
		// 人数選択画像表示
		DrawRotaGraph(Application::SCREEN_SIZE_X / 2, Application::SCREEN_SIZE_Y / 2 + 100, 2.0, 0.0, selectImg_[SELECT::SELECT_1P], TRUE);
		break;
	case PlayerNumScene::SELECT_2P:
		// 人数選択画像表示
		DrawRotaGraph(Application::SCREEN_SIZE_X / 2, Application::SCREEN_SIZE_Y / 2 + 100, 2.0, 0.0, selectImg_[SELECT::SELECT_2P], TRUE);
		break;
	case PlayerNumScene::SELECT_3P:
		// 人数選択画像表示
		DrawRotaGraph(Application::SCREEN_SIZE_X / 2, Application::SCREEN_SIZE_Y / 2 + 100, 2.0, 0.0, selectImg_[SELECT::SELECT_3P], TRUE);
		break;
	case PlayerNumScene::SELECT_4P:
		// 人数選択画像表示
		DrawRotaGraph(Application::SCREEN_SIZE_X / 2, Application::SCREEN_SIZE_Y / 2 + 100, 2.0, 0.0, selectImg_[SELECT::SELECT_4P], TRUE);
		break;
	case PlayerNumScene::SELECT_MAX:
		break;
	default:
		break;
	}

	// 操作ガイド
	/*DrawFormatString2(500, 170, GetColor(180, 180, 255), -1, "PAD1: X=左 / B=右 / A=決定 / Y=戻る");
	DrawFormatString2(500, 210, GetColor(180, 180, 255), -1, "KEY : ←/A/↑/W=左, →/D/↓/S=右, Enter/Space=決定, B=戻る");*/
}

void PlayerNumScene::Release(void)
{
	// 画像解放
	if (bgImg_ != -1)
	{
		DeleteGraph(bgImg_);
		bgImg_ = -1;
	}
	for (int i = 0; i < SELECT::SELECT_MAX; i++)
	{
		if (selectImg_[i] != -1)
		{
			DeleteGraph(selectImg_[i]);
			selectImg_[i] = -1;
		}
	}
	if (selectPromptImg_ != -1)
	{
		DeleteGraph(selectPromptImg_);
		selectPromptImg_ = -1;
	}
}
