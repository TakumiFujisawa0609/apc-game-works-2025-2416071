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
	// カメラを固定
	Camera* camera = SceneManager::GetInstance().GetCamera();
	camera->ChangeMode(Camera::MODE::FIXED_POINT);

	// プレイヤー人数取得
	playerNum_ = SceneManager::GetInstance().GetPlayerNum() + 1;

	// グリッド初期化
	/*grid_ = new Grid();
	grid_->Init();*/

	// ステージ初期化
	Stage::GetInstance().Init();

	// プレイヤーマネージャー初期化
	playerManager_ = &PlayerManager::GetInstance();
	playerManager_->Reset();

	// CharacterSelect で選択されたタイプを取得
	const auto& selected = SceneManager::GetInstance().GetSelectedPlayerNums();

	if (!selected.empty())
	{
		playerNum_ = static_cast<int>(selected.size());
		if (playerNum_ < 1) playerNum_ = 1;
		if (playerNum_ > 4) playerNum_ = 4;
	}

	// プレイヤー生成
	for (int i = 0; i < playerNum_; ++i)
	{
		// タイプ決定
		PlayerType type;
		if (!selected.empty())
		{
			int ti = selected[i];
			if (ti < 0) ti = 0;
			if (ti > 3) ti = 3;
			type = static_cast<PlayerType>(ti);
		}
		else
		{
			type = static_cast<PlayerType>(i); // 従来の挙動
		}

		// タイプに応じてパラメータを設定
		PlayerParam param;
		switch (type)
		{
		case PlayerType::Player_1: param.weight = 10.0f; param.speed = 4.0f; param.jumpPower = 7.0f; break;
		case PlayerType::Player_2: param.weight = 10.0f; param.speed = 4.0f; param.jumpPower = 5.0f; break;
		case PlayerType::Player_3: param.weight = 10.0f; param.speed = 4.0f; param.jumpPower = 2.0f; break;
		case PlayerType::Player_4: param.weight = 10.0f; param.speed = 4.0f; param.jumpPower = 2.0f; break;
		default:				   param.weight = 10.0f; param.speed = 4.0f; param.jumpPower = 5.0f; break;
		}

		playerManager_->CreatePlayer(type, i, param);
	}

	// 初期化
	playerManager_->InitAllPlayers();

	// 遷移タイマーリセット
	transitionTimer_ = 0;
}

void GameScene::Update()
{
	InputManager& ins = InputManager::GetInstance();

	// グリッド更新
	//grid_->Update();
	
	// プレイヤー更新とステージ傾き更新を一本化
	playerManager_->UpdatePlayers(Stage::GetInstance());

	// ゲームオーバー状態を取得
	bool isGameOver = playerManager_->GetIsGameOver();

	// ゲームオーバー時、数秒後にリザルト画面へ遷移
	if (isGameOver)
	{
		transitionTimer_++;

		// 3秒後にリザルト画面へ遷移
		if (transitionTimer_ > 180)
		{
			SceneManager::GetInstance().ChangeScene(SceneManager::SCENE_ID::RESULT);
		}
	}
}

void GameScene::Draw()
{
	// グリッド描画
	//grid_->Draw();

	// ステージ描画
	Stage::GetInstance().Draw();

	// プレイヤー描画
	playerManager_->DrawPlayers();

	// 操作説明
	if (playerManager_->GetIsGameOver())
	{
		int winnerID = playerManager_->GetWinnerID();
		char buffer[128];
		sprintf_s(buffer, "Player %d Wins!", winnerID + 1);
		// PAD1の右ボタンまたはEnterキーでタイトルへ戻る 操作促し文字
		DrawString(100, 200, "終了！！　自動で画面が変わります", GetColor(250, 130, 130));
	}

	// デバッグ：プレイヤー人数表示
	DrawFormatString(10, 10, GetColor(255, 255, 255), "Player Num: %d", playerNum_);

	// デバッグ：勝者表示
	if (playerManager_->GetIsGameOver())
	{
		int winnerID = playerManager_->GetWinnerID();
		DrawFormatString(10, 30, GetColor(255, 255, 0), "Winner: Player %d", winnerID + 1);
	}

}

void GameScene::Draw3D()
{
	// 3D描画例（デバッグ）
	//DrawSphere3D({ 0.f, 100.f, 0.f }, 50.f, 32, GetColor(255, 0, 0), 0xFF00FF, FALSE);
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

