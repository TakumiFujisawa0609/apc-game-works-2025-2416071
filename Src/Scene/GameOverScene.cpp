#include "GameOverScene.h"
#include "TitleScene.h"
#include "../Manager/InputManager.h"
#include <DxLib.h>

GameOverScene::GameOverScene()
{

}

GameOverScene::~GameOverScene()
{

}

void GameOverScene::Init()
{
	nextScene_ = false;
}

void GameOverScene::Update()
{
	if (InputManager::IsTrigger(KEY_INPUT_SPACE)) {
		nextScene_ = true;
	}
}

void GameOverScene::Draw()
{
	DrawString(100, 200, "ゲームオーバー", GetColor(255, 0, 0));
}

void GameOverScene::End()
{
	//画像や関数の解放は絶対にすること
}

SceneBase* GameOverScene::NextScene()
{
	if (nextScene_) return new TitleScene();
	return nullptr;
}
