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

	// 追加: 実際にスポーンしたタイプ一覧（ID順）を記録
	std::vector<int> spawnedTypes;
	spawnedTypes.reserve(createCount);

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

		// 記録（ID=i のタイプ）
		spawnedTypes.push_back(static_cast<int>(type));

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

	// Result で参照できるよう SceneManager に保存
	SceneManager::GetInstance().SetLastSpawnedTypes(spawnedTypes);

	playerManager_->InitAllPlayers();
	transitionTimer_ = 0;
}

void GameScene::Update()
{
	InputManager& ins = InputManager::GetInstance();

	// プレイヤー/ステージ更新
	playerManager_->UpdatePlayers(Stage::GetInstance());

	// 1人プレイ（ID=0 が人間）: 死亡したら即ゲームオーバー
	if (singlePlayerMode_ && !playerManager_->GetIsGameOver())
	{
		const auto raw = playerManager_->GetPlayerRawPlayers();
		for (auto* p : raw) {
			if (!p) continue;
			if (p->GetID() == 0) {
				if (!p->IsAlive()) {
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
		if (transitionTimer_ > 90)
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

	// デバッグ
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

void GameScene::DrawPlayerMarkers3D()
{
	const auto raw = playerManager_->GetPlayerRawPlayers();
	for (auto* p : raw)
	{
		if (!p || !p->IsAlive()) continue;

		const int pid = p->GetID();
		const bool isHuman = (singlePlayerMode_ && pid == humanPlayerId_);
		const char* label = isHuman ? "YOU" : (singlePlayerMode_ ? "CPU" : "");

		// マルチ時（singlePlayerMode_ == false）はラベル表示なし（必要なら P1/P2 に変更）
		if (!singlePlayerMode_ && label[0] == '\0') continue;

		VECTOR pos = p->GetPos();
		VECTOR markerPos = VGet(pos.x, pos.y + 140.0f, pos.z);

		bool highlight = isHuman && (youMarkerTimer_ < YOU_MARKER_HIGHLIGHT_FRAMES) && ((youMarkerTimer_ / 8) % 2 == 0);
		unsigned int colTri = highlight ? GetColor(255, 255, 0) : GetColor(180, 220, 255);
		unsigned int colText = highlight ? GetColor(255, 255, 120) : GetColor(220, 240, 255);

		// 下向き矢印（三角）：DxLib は塗りつぶしフラグが必要（TRUE）
		DrawTriangle3D(
			VGet(markerPos.x - 20.0f, markerPos.y + 20.0f, markerPos.z),
			VGet(markerPos.x + 20.0f, markerPos.y + 20.0f, markerPos.z),
			VGet(markerPos.x, markerPos.y, markerPos.z),
			colTri,
			TRUE);

		// ラベル（YOU / CPU）を頭上に 2D 表示
		DrawLabelAtWorld(label, VGet(markerPos.x, markerPos.y + 50.0f, markerPos.z), colText);
	}
}

void GameScene::DrawPlayerHUDLegend()
{
	unsigned int col = (youMarkerTimer_ < YOU_MARKER_HIGHLIGHT_FRAMES) ? GetColor(255, 255, 120) : GetColor(200, 220, 255);
	if (singlePlayerMode_)
	{
		//DrawFormatString2(20, 50, col, -1, "YOU: あなたの操作キャラ");
		//DrawFormatString2(20, 70, GetColor(180, 200, 255), -1, "CPU: コンピュータ操作キャラ");
	}
	else
	{
		//DrawFormatString2(20, 50, GetColor(200, 220, 255), -1, "各プレイヤーの頭上にマーカーを表示");
	}
}


void GameScene::DrawLabelAtWorld(const char* text, const VECTOR& worldPos, unsigned int color)
{
	VECTOR screenPos = ConvWorldPosToScreenPos(worldPos);

	// 画面座標へ変換後に文字を描画（適宜オフセット調整）
	DrawFormatString2((int)screenPos.x - 16, (int)screenPos.y - 8, color, -1, text);
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