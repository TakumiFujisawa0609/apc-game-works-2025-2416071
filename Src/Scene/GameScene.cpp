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
	// カメラを固定
	Camera* camera = SceneManager::GetInstance().GetCamera();
	camera->ChangeMode(Camera::MODE::FIXED_POINT);

	// プレイヤー人数取得
	int scenePlayerNum = SceneManager::GetInstance().GetPlayerNum();

	// Stage 初期化
	Stage::GetInstance().Init();

	// PlayerManager 初期化
	playerManager_ = &PlayerManager::GetInstance();
	playerManager_->Reset();

	// CharacterSelect で選ばれたキャラIDリスト（各プレイヤーの選択）
	const auto& selected = SceneManager::GetInstance().GetSelectedPlayerNums();

	// createCount: GameScene で生成するプレイヤー（実体）の数
	int createCount = scenePlayerNum;
	bool singlePlayerMode = (scenePlayerNum == 1);

	if (singlePlayerMode)
	{
		createCount = 4; // 1人選択時のみ 4体に拡張
	}
	else
	{
		// selected に値が入っていればその数を優先して使う
		if (!selected.empty())
		{
			createCount = static_cast<int>(selected.size());
		}
		// safety clamp
		if (createCount < 1) createCount = 1;
		if (createCount > 4) createCount = 4;
	}

	// プレイヤー生成ループ
	for (int i = 0; i < createCount; ++i)
	{
		PlayerType type = PlayerType::Player_1;

		// キャラタイプ決定
		if (!selected.empty())
		{
			// singlePlayerMode のとき selected は通常サイズ1（選んだキャラ）
			// i==0 はプレイヤーの選択キャラ、i>0 は AI 用に順次別のキャラを割り当てる
			if (singlePlayerMode)
			{
				int humanTypeIndex = selected[0];
				int t = (humanTypeIndex + i) % 4;
				if (t < 0) t = 0;
				type = static_cast<PlayerType>(t);
			}
			else
			{
				// マルチプレイ（選択された複数）: i 番目に選ばれたキャラを使う
				int ti = selected[i];
				if (ti < 0) ti = 0;
				if (ti > 3) ti = 3;
				type = static_cast<PlayerType>(ti);
			}
		}
		else
		{
			// 選択がない場合は既存の安定的割当（i % 4）
			int t = i % 4;
			type = static_cast<PlayerType>(t);
		}

		// プレイヤーパラメータ設定（既存ロジックを維持）
		PlayerParam param;
		switch (type)
		{
		case PlayerType::Player_1: param.weight = 10.0f; param.speed = 4.0f; param.jumpPower = 7.0f; break;
		case PlayerType::Player_2: param.weight = 10.0f; param.speed = 4.0f; param.jumpPower = 5.0f; break;
		case PlayerType::Player_3: param.weight = 10.0f; param.speed = 4.0f; param.jumpPower = 2.0f; break;
		case PlayerType::Player_4: param.weight = 10.0f; param.speed = 4.0f; param.jumpPower = 2.0f; break;
		default:                   param.weight = 10.0f; param.speed = 4.0f; param.jumpPower = 5.0f; break;
		}

		// createAsHuman 判定
		bool createAsHuman = false;
		if (singlePlayerMode)
		{
			// single-player の場合は i==0 が人間、それ以外は AI
			createAsHuman = (i == 0);
		}
		else
		{
			// マルチプレイ（2P/3P/4P）はすべて人間（AI は生成しない）
			createAsHuman = true;
		}

		if (createAsHuman)
		{
			// 既存の CreatePlayer（内部で Controller を作る）を利用
			playerManager_->CreatePlayer(type, i, param);
		}
		else
		{
			// single-player の AI を生成して注入（ownerId=i）
			std::unique_ptr<InputController> aiController = std::make_unique<AIController>(i);
			playerManager_->CreatePlayer(type, i, param, std::move(aiController));
		}
	}

	// 生成したプレイヤーを初期化（ループ外で一度だけ呼ぶ）
	playerManager_->InitAllPlayers();

	// トランジションタイマー初期化
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
		//sprintf_s(buffer, "Player %d Wins!", winnerID + 1);
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
	// 3D描画が必要な場合はここに追加
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

