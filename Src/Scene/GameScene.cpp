#include <cmath>
#include <DxLib.h>
#include "../Manager/InputManager.h"
#include "../Manager/SceneManager.h"
#include "../Object/Grid.h"
#include "../Manager/Camera.h"
#include "../Object/Player/Common/PlayerManager.h"
#include "../Object/Stage/Stage.h"
#include "GameScene.h"
#include "../Object/Player/Player/Player_1.h"
#include "../Object/Player/Control/AIController.h"

GameScene::GameScene()
{
	//grid_ = nullptr;
	playerManager_ = nullptr;
}

GameScene::~GameScene()
{
}

void GameScene::Init()
{
	// カメラを安定リセット
	Camera* camera = SceneManager::GetInstance().GetCamera();
	camera->ChangeMode(Camera::MODE::FIXED_POINT);
	camera->ResetForGame(30.0f, 1400.0f, true, 1.5f);

	// プレイヤー数
	playerNum_ = SceneManager::GetInstance().GetPlayerNum();
	singlePlayerMode_ = (playerNum_ == 1);

	// ステージ
	Stage::GetInstance().Init();

	// PlayerManager
	playerManager_ = &PlayerManager::GetInstance();
	playerManager_->Reset();

	// 選択キャラ
	const auto& selected = SceneManager::GetInstance().GetSelectedPlayerNums();

	// 生成数
	int createCount = playerNum_;
	if (singlePlayerMode_) {
		createCount = 4; // 1人選択でも4体生成（0は人間、他はAI）
	}
	else {
		if (!selected.empty()) createCount = (int)selected.size();
		if (createCount < 1) createCount = 1;
		if (createCount > 4) createCount = 4;
	}

	for (int i = 0; i < createCount; ++i)
	{
		PlayerType type = PlayerType::Player_1;
		if (!selected.empty())
		{
			if (singlePlayerMode_) {
				int humanTypeIndex = selected[0];
				type = static_cast<PlayerType>((humanTypeIndex + i) % 4);
			}
			else {
				int ti = selected[i]; if (ti < 0) ti = 0; if (ti > 3) ti = 3;
				type = static_cast<PlayerType>(ti);
			}
		}
		else {
			type = static_cast<PlayerType>(i % 4);
		}

		PlayerParam param;
		switch (type)
		{
		case PlayerType::Player_1: param.weight = 10.0f; param.speed = 4.0f; param.jumpPower = 7.0f; break;
		case PlayerType::Player_2: param.weight = 10.0f; param.speed = 4.0f; param.jumpPower = 5.0f; break;
		case PlayerType::Player_3: param.weight = 10.0f; param.speed = 4.0f; param.jumpPower = 2.0f; break;
		case PlayerType::Player_4: param.weight = 10.0f; param.speed = 4.0f; param.jumpPower = 2.0f; break;
		default:                   param.weight = 10.0f; param.speed = 4.0f; param.jumpPower = 5.0f; break;
		}

		bool createAsHuman = singlePlayerMode_ ? (i == 0) : true;

		if (createAsHuman) {
			playerManager_->CreatePlayer(type, i, param);
		}
		else {
			std::unique_ptr<InputController> aiController = std::make_unique<AIController>(i);
			playerManager_->CreatePlayer(type, i, param, std::move(aiController));
		}
	}

	playerManager_->InitAllPlayers();
	transitionTimer_ = 0;
}

void GameScene::Update()
{
	InputManager& ins = InputManager::GetInstance();

	// プレイヤー/ステージ更新
	playerManager_->UpdatePlayers(Stage::GetInstance());

	// 1人プレイ時、ID=0（人間）が死亡したら即ゲームオーバー
	if (singlePlayerMode_ && !playerManager_->GetIsGameOver())
	{
		const auto raw = playerManager_->GetPlayerRawPlayers();
		for (auto* p : raw) {
			if (!p) continue;
			if (p->GetID() == 0) {
				if (!p->IsAlive()) {
					// 勝者未設定（-1）でゲームオーバー
					playerManager_->ForceGameOver(-1);
				}
				break;
			}
		}
	}

	// ゲームオーバー → 結果へ
	if (playerManager_->GetIsGameOver())
	{
		transitionTimer_++;
		if (transitionTimer_ > 90) // 1.5秒ほど待ってから
		{
			SceneManager::GetInstance().ChangeScene(SceneManager::SCENE_ID::RESULT);
		}
	}
}

void GameScene::Draw()
{
	// ステージ
	Stage::GetInstance().Draw();

	// プレイヤー
	playerManager_->DrawPlayers();

	// デバッグ表示
	DrawFormatString(10, 10, GetColor(255, 255, 255), "Player Num: %d", playerNum_);

	if (playerManager_->GetIsGameOver())
	{
		if (playerNum_ == 1)
		{
			DrawString(100, 200, "GAME OVER", GetColor(250, 130, 130));
		}
		else
		{
			int winnerID = playerManager_->GetWinnerID();
			DrawFormatString(100, 200, GetColor(255, 255, 0), "Winner: Player %d", winnerID + 1);
		}
	}
}

void GameScene::Draw3D()
{
}

void GameScene::Release()
{
	Stage::GetInstance().Release();

	if (playerManager_)
	{
		playerManager_->ClearPlayers();
		playerManager_ = nullptr;
	}
}