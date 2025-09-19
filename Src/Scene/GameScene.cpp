#include <cmath>
#include <DxLib.h>
#include "../Manager/InputManager.h"
#include "../Manager/SceneManager.h"
#include "../Object/Grid.h"
#include "../Manager/Camera.h"
#include "../Object/Stage/StageManager.h"
#include "../Object/Player/PlayerManager.h"
#include "GameScene.h"

GameScene::GameScene()
{
	// ぬるぽ
	grid_ = nullptr;
}

GameScene::~GameScene()
{
}

void GameScene::Init()
{
	// いったんカメラを固定にする
	Camera* camera = SceneManager::GetInstance().GetCamera();
	camera->ChangeMode(Camera::MODE::FIXED_POINT);

	// プレイヤー人数取得
	playerNum_ = SceneManager::GetInstance().GetPlayerNum() + 1;

	// グリッド初期化
	grid_ = new Grid();
	grid_->Init();

	// プレイヤーマネージャー初期化
	playerManager_ = new PlayerManager();
	playerManager_->InitAllPlayers();

}

void GameScene::Update()
{
	InputManager& ins = InputManager::GetInstance();

	// グリッド更新
	grid_->Update();

	// プレイヤーの人数を取得
	playerNum_ = SceneManager::GetInstance().GetPlayerNum() + 1;

	// プレイヤー更新

	if (playerNum_ == 1)
	{
		playerManager_->CreatePlayer(CharacterType::Player_1, 0);
		playerNum_ = 1;
	}
	else if (playerNum_ == 2)
	{
		playerManager_->CreatePlayer(CharacterType::Player_1, 0);
		playerManager_->CreatePlayer(CharacterType::Player_2, 1);
		playerNum_ = 2;
	}

	// シーン遷移
	
	if (ins.IsTrgDown(KEY_INPUT_SPACE))
	{
		// 実装後にデバッグをすること。
		// 例外スロー起きます
		SceneManager::GetInstance().ChangeScene(SceneManager::SCENE_ID::TITLE);
	}
}

void GameScene::Draw()
{
	// グリッド描画
	grid_->Draw();

	// プレイヤー描画
	playerManager_->DrawAllPlayers();

	DrawFormatString2(100, 100, GetColor(255, 255, 255), -1, "Game Scene");

	// 背景色の水色は絶対に見せてはならないので、背景画像は描画必須
}

void GameScene::Draw3D()
{
	// 3D描画
	DrawSphere3D({ 0.f, 100.f, 0.f }, 50.f, 32, GetColor(255, 0, 0),0xFF00FF,FALSE);

	// ステートパターンでキャラクターを描画
	// ステージもステートパターンで描画

}

void GameScene::Release()
{
	// グリッド解放
	grid_->Release();
	delete grid_;
}
