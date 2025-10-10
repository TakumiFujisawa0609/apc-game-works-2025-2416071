#include <cmath>
#include <DxLib.h>
#include "../Manager/InputManager.h"
#include "../Manager/SceneManager.h"
#include "../Object/Grid.h"
#include "../Manager/Camera.h"
#include "../Object/Player/PlayerManager.h"
#include "../Object/Stage/Stage.h"
#include "GameScene.h"
#include "../Object/Player/Player_1.h"

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
	Stage::GetInstance().Init();

	// プレイヤーマネージャー初期化
	playerManager_ = &PlayerManager::GetInstance();


	// プレイヤー生成（デバッグ用を含め、ID:0 から生成）
	for (int i = 0; i < playerNum_; ++i)
	{

		// Playerも変わってほしいので、ここでタイプ変更
		PlayerType type = static_cast<PlayerType>(i);

		// InputIdもIDと同じにする
		int inputId = i;

		playerManager_->CreatePlayer(type, i, 15.0f,inputId);
	}

	// プレイヤー初期化
	playerManager_->InitAllPlayers();
}

void GameScene::Update()
{
	InputManager& ins = InputManager::GetInstance();

	// グリッド更新
	grid_->Update();
	
	// プレイヤー更新とステージ傾き更新を一本化
	playerManager_->UpdatePlayers(Stage::GetInstance());

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
	playerManager_->DrawPlayers();

	// ステージ描画
	Stage::GetInstance().Draw(); // ★ シングルトンのDrawを呼ぶ

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

	Stage::GetInstance().Release();

	if (playerManager_)
	{
		playerManager_->ClearPlayers();
		playerManager_ = nullptr;
	}

}
