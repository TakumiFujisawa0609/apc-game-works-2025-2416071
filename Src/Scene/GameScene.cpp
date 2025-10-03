#include <cmath>
#include <DxLib.h>
#include "../Manager/InputManager.h"
#include "../Manager/SceneManager.h"
#include "../Object/Grid.h"
#include "../Manager/Camera.h"
#include "../Object/Stage/StageManager.h"
#include "../Object/Player/PlayerManager.h"
#include "../Object/Stage/Stage.h"
#include "GameScene.h"

GameScene::GameScene()
{
	grid_ = nullptr;
	playerManager_ = nullptr;
}

GameScene::~GameScene()
{
}

void GameScene::Init()
{
	// カメラを固定
	Camera* camera = SceneManager::GetInstance().GetCamera();
	camera->ChangeMode(Camera::MODE::FREE);

	// プレイヤー人数取得
	playerNum_ = SceneManager::GetInstance().GetPlayerNum() + 1;

	// グリッド初期化
	grid_ = new Grid();
	grid_->Init();

	// ステージ初期化
	stage_.Init();

	// プレイヤーマネージャー初期化
	playerManager_ = &PlayerManager::GetInstance();

	// プレイヤー生成
	if (playerNum_ >= 1) playerManager_->CreatePlayer(CharacterType::Player_1, 0);
	if (playerNum_ >= 2) playerManager_->CreatePlayer(CharacterType::Player_2, 1);
	if (playerNum_ >= 3) playerManager_->CreatePlayer(CharacterType::Player_3, 2);
	if (playerNum_ >= 4) playerManager_->CreatePlayer(CharacterType::Player_4, 3);

	// プレイヤー初期化
	playerManager_->InitAllPlayers();
}

void GameScene::Update()
{
	InputManager& ins = InputManager::GetInstance();

	// グリッド更新
	grid_->Update();

	// プレイヤー更新
	playerManager_->UpdateAllPlayers(stage_);

	// ステージ更新
	stage_.Update();

	// シーン遷移（デバッグ用）
	if (ins.IsTrgDown(KEY_INPUT_SPACE))
	{
		SceneManager::GetInstance().ChangeScene(SceneManager::SCENE_ID::TITLE);
	}
}

void GameScene::Draw()
{
	// グリッド描画
	grid_->Draw();

	// プレイヤー描画
	playerManager_->DrawAllPlayers();

	// ステージ描画
	stage_.Draw();

	DrawFormatString2(100, 100, GetColor(255, 255, 255), -1, "Game Scene");
}

void GameScene::Draw3D()
{
	// 3D描画例（デバッグ）
	DrawSphere3D({ 0.f, 100.f, 0.f }, 50.f, 32, GetColor(255, 0, 0), 0xFF00FF, FALSE);
}

void GameScene::Release()
{
	if (grid_)
	{
		grid_->Release();
		delete grid_;
		grid_ = nullptr;
	}

	stage_.Release();

	if (playerManager_)
	{
		playerManager_->ClearPlayers();
		playerManager_ = nullptr;
	}
}
