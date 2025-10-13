#include <DxLib.h>
#include "../Manager/InputManager.h"
#include "../Manager/SceneManager.h"
#include "ManualScene.h"

ManualScene::ManualScene(void)
{
}

ManualScene::~ManualScene(void)
{
}

void ManualScene::Init(void)
{
}

void ManualScene::Update(void)
{
	// シーン遷移
	InputManager& ins = InputManager::GetInstance();
	if (ins.IsTrgDown(KEY_INPUT_A))
	{
		// 実装後にデバッグをすること。
		// 例外スロー起きます
		SceneManager::GetInstance().ChangeScene(SceneManager::SCENE_ID::PLAYERNUMBERSELECT);
	}
	else if (ins.IsTrgDown(KEY_INPUT_B))
	{
		SceneManager::GetInstance().ChangeScene(SceneManager::SCENE_ID::TITLE);
	}
}

void ManualScene::Draw(void)
{
	DrawFormatString(100, 100, GetColor(255, 255, 255), "操作説明画面");
	DrawFormatString(100, 140, GetColor(255, 255, 255), "Aキーでプレイヤー人数選択");
	DrawFormatString(100, 180, GetColor(255, 255, 255), "Bキーでタイトル");
}

void ManualScene::Release(void)
{
}
