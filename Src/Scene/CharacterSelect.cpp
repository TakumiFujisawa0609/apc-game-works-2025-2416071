#include "../Manager/InputManager.h"
#include "../Manager/SceneManager.h"
#include "PlayerNumScene.h"
#include "CharacterSelect.h"

CharacterSelect::CharacterSelect(void)
{
}


CharacterSelect::~CharacterSelect(void)
{
}

void CharacterSelect::Init(void)
{
	playerNum_ = SceneManager::GetInstance().GetPlayerNum() + 1;
}

void CharacterSelect::Update(void)
{
	// シーン遷移
	InputManager& ins = InputManager::GetInstance();
	SceneManager& scene = SceneManager::GetInstance();
	if (ins.IsTrgDown(KEY_INPUT_B))
	{
		// 実装後にデバッグをすること。
		// 例外スロー起きます
		scene.ChangeScene(SceneManager::SCENE_ID::PLAYERNUMBERSELECT);
	}
	else if (ins.IsTrgDown(KEY_INPUT_A))
	{
		scene.ChangeScene(SceneManager::SCENE_ID::GAME);
	}

	//プレイヤー人数によって処理を分ける
	switch (playerNum_)
	{
	case 1:
		//1人用の処理
		break;
	default:
		//2人以上用の処理
		break;
	}
}

void CharacterSelect::Draw(void)
{
	// キャラ選択表示
	DrawFormatString2(100, 100, GetColor(255, 255, 255), -1, "Character Select Scene");
	DrawFormatString2(100, 140, GetColor(255, 255, 255), -1, "A to GAME");
	DrawFormatString2(100, 180, GetColor(255, 255, 255), -1, "B to PLAYERNUMBERSELECT");

	DrawFormatString(100, 220, GetColor(255, 255, 255), "人数: %d", playerNum_);
}

void CharacterSelect::Release(void)
{
	delete playerNumSelect_;
}
