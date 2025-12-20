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
	// カメラをゲーム用設定に変更
	Camera* camera = SceneManager::GetInstance().GetCamera();
	camera->ChangeMode(Camera::MODE::FIXED_POINT);
	camera->ResetForGame(30.0f, 1400.0f, true, 1.5f);

	// プレイヤー数の取得
	playerNum_ = SceneManager::GetInstance().GetPlayerNum();
	singlePlayerMode_ = (playerNum_ == 1);

	// ステージ初期化
	Stage::GetInstance().Init();

	// PlayerManager
	playerManager_ = &PlayerManager::GetInstance();
	playerManager_->Reset();

	// キャラ選択から受け取った選択タイプ
	const auto& selected = SceneManager::GetInstance().GetSelectedPlayerNums();

	// 生成数の決定
	int createCount = playerNum_;
	if (singlePlayerMode_) {
		createCount = 4; // シングル時は4体（ID0が人間、他はAI）
	}
	else {
		if (!selected.empty()) createCount = (int)selected.size();
		if (createCount < 1) createCount = 1;
		if (createCount > 4) createCount = 4;
	}

	// Result 用にスポーンしたタイプを記録
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

		// 記録
		spawnedTypes.push_back(static_cast<int>(type));

		// デフォルトパラメータ（簡易）
		PlayerParam param;
		switch (type)
		{
		case PlayerType::Player_1: param.weight = 10.0f; param.speed = 4.0f; param.jumpPower = 7.0f; break;
		case PlayerType::Player_2: param.weight = 10.0f; param.speed = 4.0f; param.jumpPower = 5.0f; break;
		case PlayerType::Player_3: param.weight = 10.0f; param.speed = 4.0f; param.jumpPower = 2.0f; break;
		case PlayerType::Player_4: param.weight = 10.0f; param.speed = 4.0f; param.jumpPower = 2.0f; break;
		default:                   param.weight = 10.0f; param.speed = 4.0f; param.jumpPower = 5.0f; break;
		}

		// シングル時のみ ID0 を人間、それ以外はAI
		bool createAsHuman = singlePlayerMode_ ? (i == 0) : true;

		if (createAsHuman) {
			playerManager_->CreatePlayer(type, i, param);
		}
		else {
			std::unique_ptr<InputController> aiController = std::make_unique<AIController>(i);
			playerManager_->CreatePlayer(type, i, param, std::move(aiController));
		}
	}

	// Result シーンで使うため保存
	SceneManager::GetInstance().SetLastSpawnedTypes(spawnedTypes);

	playerManager_->InitAllPlayers();
	transitionTimer_ = 0;
}

void GameScene::Update()
{
	InputManager& ins = InputManager::GetInstance();

	// プレイヤー／ステージ更新
	playerManager_->UpdatePlayers(Stage::GetInstance());

	// シングルプレイ時：人間（ID=0）が死んだらゲームオーバー
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

	// ゲームオーバー時は一定時間後にリザルトへ遷移
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

	// 追加：生存状況HUD
	DrawPlayerStatusHUD();

	// デバッグ表示（任意）
	//DrawFormatString(10, 10, GetColor(255, 255, 255), "Player Num: %d", playerNum_);

	// ゲームオーバー表示
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

		// マルチプレイでは空ラベルは表示しない
		if (!singlePlayerMode_ && label[0] == '\0') continue;

		VECTOR pos = p->GetPos();
		VECTOR markerPos = VGet(pos.x, pos.y + 140.0f, pos.z);

		bool highlight = isHuman && (youMarkerTimer_ < YOU_MARKER_HIGHLIGHT_FRAMES) && ((youMarkerTimer_ / 8) % 2 == 0);
		unsigned int colTri = highlight ? GetColor(255, 255, 0) : GetColor(180, 220, 255);
		unsigned int colText = highlight ? GetColor(255, 255, 120) : GetColor(220, 240, 255);

		// 三角形（頭上の目印）を描画
		DrawTriangle3D(
			VGet(markerPos.x - 20.0f, markerPos.y + 20.0f, markerPos.z),
			VGet(markerPos.x + 20.0f, markerPos.y + 20.0f, markerPos.z),
			VGet(markerPos.x, markerPos.y, markerPos.z),
			colTri,
			TRUE);

		// ラベル（YOU/CPU）を2Dで重ねて表示
		DrawLabelAtWorld(label, VGet(markerPos.x, markerPos.y + 50.0f, markerPos.z), colText);
	}
}

void GameScene::DrawPlayerHUDLegend()
{
	unsigned int col = (youMarkerTimer_ < YOU_MARKER_HIGHLIGHT_FRAMES) ? GetColor(255, 255, 120) : GetColor(200, 220, 255);
	// HUDの凡例を表示する場合はここに描画を書く（現状未使用）
}

void GameScene::DrawLabelAtWorld(const char* text, const VECTOR& worldPos, unsigned int color)
{
	VECTOR screenPos = ConvWorldPosToScreenPos(worldPos);
	DrawFormatString((int)screenPos.x - 16, (int)screenPos.y - 8, color, "%s", text);
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

// 追加：プレイヤー生存状態HUD（画面左上）
// - Pn と YOU/CPU のラベルを表示
// - 生存は緑、死亡は赤で [DEAD] を付ける
// - 右端に丸アイコン（生存=緑／死亡=赤）も表示
void GameScene::DrawPlayerStatusHUD()
{
	const auto raw = playerManager_->GetPlayerRawPlayers();
	if (raw.empty()) return;

	// 半透明のパネル
	int panelLeft = 12;
	int panelTop = 12;
	int panelWidth = 260;
	int panelHeight = 28 + (int)raw.size() * 24;

	SetDrawBlendMode(DX_BLENDMODE_ALPHA, 140);
	DrawBox(panelLeft, panelTop, panelLeft + panelWidth, panelTop + panelHeight, GetColor(30, 30, 30), TRUE);
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	DrawBox(panelLeft, panelTop, panelLeft + panelWidth, panelTop + panelHeight, GetColor(80, 80, 120), FALSE);

	// 見出し
	DrawFormatString(panelLeft + 8, panelTop + 6, GetColor(220, 240, 255), "Players");

	// 各プレイヤーの状態
	int y = panelTop + 28;
	for (auto* p : raw)
	{
		if (!p) continue;
		int id = p->GetID();
		bool alive = p->IsAlive();

		unsigned int nameCol = alive ? GetColor(120, 255, 160) : GetColor(255, 120, 120);
		const char* role = "";
		if (singlePlayerMode_) role = (id == 0) ? "YOU" : "CPU";

		// ラベル（Pn / YOU or CPU / DEAD表示）
		DrawFormatString(panelLeft + 12, y, nameCol, "P%d %s %s", id + 1, role, alive ? "" : "[DEAD]");

		// 右端の丸インジケータ
		int cx = panelLeft + panelWidth - 24;
		int cy = y + 6;
		unsigned int dotCol = alive ? GetColor(100, 220, 140) : GetColor(220, 80, 80);
		DrawCircle(cx, cy, 6, dotCol, TRUE);
		DrawCircle(cx, cy, 6, GetColor(40, 40, 40), FALSE);

		y += 24;
	}
}