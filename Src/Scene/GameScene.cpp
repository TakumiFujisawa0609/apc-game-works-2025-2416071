//#include <cmath>
//#include <DxLib.h>
//#include "../Manager/InputManager.h"
//#include "../Manager/SceneManager.h"
//#include "../Object/Grid.h"
//#include "../Manager/Camera.h"
//#include "../Object/Stage/StageManager.h"
//#include "../Object/Player/PlayerManager.h"
//#include "../Object/Stage/Stage.h"
//#include "GameScene.h"
//#include "../Object/Player/Player_1.h"
//
//GameScene::GameScene()
//{
//	grid_ = nullptr;
//	playerManager_ = nullptr;
//}
//
//GameScene::~GameScene()
//{
//}
//
//void GameScene::Init()
//{
//	// カメラを固定
//	Camera* camera = SceneManager::GetInstance().GetCamera();
//	camera->ChangeMode(Camera::MODE::FREE);
//
//	// プレイヤー人数取得
//	playerNum_ = SceneManager::GetInstance().GetPlayerNum() + 1;
//
//	// グリッド初期化
//	grid_ = new Grid();
//	grid_->Init();
//
//	// ステージ初期化
//	stage_.Init();
//
//	// プレイヤーマネージャー初期化
//	playerManager_ = &PlayerManager::GetInstance();
//
//	// プレイヤー
//	debugPlayer_ = new Player_1(0, 1.0f);
//	debugPlayer_->Init();
//
//	// プレイヤー生成
//	for (int i = 0; i < playerNum_; ++i)
//	{
//		playerManager_->CreatePlayer(static_cast<PlayerType>(i), i, 1.0f);
//	}
//
//	// プレイヤー初期化
//	playerManager_->InitAllPlayers();
//}
//
//void GameScene::Update()
//{
//	InputManager& ins = InputManager::GetInstance();
//
//	// グリッド更新
//	grid_->Update();
//
//	// デバッグ用プレイヤー更新
//	debugPlayer_->Update();
//
//	// プレイヤー更新
//	playerManager_->UpdatePlayers(stage_);
//
//	// ステージ更新
//	stage_.Update();
//
//	// シーン遷移（デバッグ用）
//	if (ins.IsTrgDown(KEY_INPUT_SPACE))
//	{
//		SceneManager::GetInstance().ChangeScene(SceneManager::SCENE_ID::TITLE);
//	}
//}
//
//void GameScene::Draw()
//{
//	// グリッド描画
//	grid_->Draw();
//
//	// プレイヤー描画
//	playerManager_->DrawPlayers();
//
//	// ステージ描画
//	stage_.Draw();
//
//	DrawFormatString2(100, 100, GetColor(255, 255, 255), -1, "Game Scene");
//
//	// デバッグ用にプレイヤー情報表示
//	debugPlayer_->DebugDraw();
//	
//}
//
//void GameScene::Draw3D()
//{
//	// 3D描画例（デバッグ）
//	DrawSphere3D({ 0.f, 100.f, 0.f }, 50.f, 32, GetColor(255, 0, 0), 0xFF00FF, FALSE);
//}
//
//void GameScene::Release()
//{
//	if (grid_)
//	{
//		grid_->Release();
//		delete grid_;
//		grid_ = nullptr;
//	}
//
//	stage_.Release();
//
//	if (playerManager_)
//	{
//		playerManager_->ClearPlayers();
//		playerManager_ = nullptr;
//	}
//}


// GameScene.cpp 修正版

#include <cmath>
#include <DxLib.h>
#include "../Manager/InputManager.h"
#include "../Manager/SceneManager.h"
#include "../Object/Grid.h"
#include "../Manager/Camera.h"
// StageManagerは使わないと判断しコメントアウト
// #include "../Object/Stage/StageManager.h" 
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
	// debugPlayer_をPlayerManagerに登録する修正をした場合、Releaseでdeleteしないように注意が必要です。
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
	// ★ ローカル変数 stage_ は削除し、シングルトンを使用
	Stage::GetInstance().Init();

	// プレイヤーマネージャー初期化
	playerManager_ = &PlayerManager::GetInstance();

	// ★ デバッグ用プレイヤーもPlayerManagerで管理し、重さの計算に含める
	// debugPlayer_ = new Player_1(0, 1.0f); // 削除

	// プレイヤー生成（デバッグ用を含め、ID:0 から生成）
	for (int i = 0; i < playerNum_; ++i)
	{
		// IDを i に統一し、重さ 1.0f で生成
		playerManager_->CreatePlayer(static_cast<PlayerType>(i), i, 1.0f);
	}

	// プレイヤー初期化
	playerManager_->InitAllPlayers();
}

void GameScene::Update()
{
	InputManager& ins = InputManager::GetInstance();

	// グリッド更新
	grid_->Update();

	// ★ デバッグ用プレイヤーの更新はPlayerManagerに任せる
	// debugPlayer_->Update(); // 削除

	// プレイヤー更新とステージ傾き更新を一本化
	// 修正: Stageインスタンスの代わりにシングルトン Stage::GetInstance() を渡す
	playerManager_->UpdatePlayers(Stage::GetInstance());
	// ※ PlayerManager::UpdatePlayers内で Stage::UpdateTilt が実行されます。

	// ★ ステージ更新 (Stage::Update) は PlayerManager::UpdatePlayers 内で
	//    既に傾きが計算されているため、ここでは不要、または PlayerManagerの後で実行
	//    Stage::Update()内から UpdateTilt を削除している前提であれば、実行しても良い
	// Stage::GetInstance().Update(); 

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

	// デバッグ用にプレイヤー情報表示
	// ★ PlayerManagerから取得するなど、表示方法を修正する必要があります
	// debugPlayer_->DebugDraw(); // 削除

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

	// ★ シングルトンのReleaseを呼ぶ
	Stage::GetInstance().Release();

	if (playerManager_)
	{
		playerManager_->ClearPlayers();
		playerManager_ = nullptr;
	}

	// ★ debugPlayer_ の解放も不要になるか、PlayerManagerに任せる
}
