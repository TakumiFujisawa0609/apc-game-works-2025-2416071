#include <cmath>
#include <DxLib.h>
#include "../Manager/InputManager.h"
#include "../Manager/SceneManager.h"
#include "../Object/Grid.h"
#include "../Manager/Camera.h"
#include "../Manager/SoundManager.h" // 追加
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
	// 画像の読み込み
	attackButtonImg_ = LoadGraph("data/Image/button/xbox_lt_outline.png");
	moveButtonImg_ = LoadGraph("data/Image/button/xbox_stick_l.png");

	// 画像描画フラグ
	showButtonHints_ = true;

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

	// SE状態トラッキング初期化（プレイヤーIDに合わせて確保）
	prevCharging_.clear();
	prevDashing_.clear();
	{
		const auto raw = playerManager_->GetPlayerRawPlayers();
		int maxId = -1;
		for (auto* p : raw) {
			if (!p) continue;
			if (p->GetID() > maxId) {
				maxId = p->GetID();
			}
		}
		if (maxId >= 0) {
			prevCharging_.assign(maxId + 1, false);
			prevDashing_.assign(maxId + 1, false);
		}
	}
}

void GameScene::Update()
{
	InputManager& ins = InputManager::GetInstance();

	// プレイヤー／ステージ更新
	playerManager_->UpdatePlayers(Stage::GetInstance());

	// チャージ/ダッシュ状態に応じたSE制御
	{
		const auto raw = playerManager_->GetPlayerRawPlayers();
		for (auto* p : raw) {
			if (!p || !p->IsAlive()) continue;
			int id = p->GetID();

			if ((int)prevCharging_.size() <= id) prevCharging_.resize(id + 1, false);
			if ((int)prevDashing_.size() <= id) prevDashing_.resize(id + 1, false);

			// チャージ保持（LT押下中）
			bool chargingNow = p->IsDashChargeHeldPublic();
			if (chargingNow && !prevCharging_[id]) {
				SoundManager::GetInstance().PlaySELoop(SE_ID::CHARGE_LOOP);
			}
			else if (!chargingNow && prevCharging_[id]) {
				SoundManager::GetInstance().StopSE(SE_ID::CHARGE_LOOP);
			}
			prevCharging_[id] = chargingNow;

			// ダッシュ開始/終了
			bool dashingNow = p->IsDashing();
			if (dashingNow && !prevDashing_[id]) {
				// 開始: 単発＋ループ開始
				SoundManager::GetInstance().PlaySE(SE_ID::DASH);
				SoundManager::GetInstance().StopSE(SE_ID::CHARGE_LOOP);
				SoundManager::GetInstance().PlaySELoop(SE_ID::DASH);
			}
			else if (!dashingNow && prevDashing_[id]) {
				// 終了: ループ停止
				SoundManager::GetInstance().StopSE(SE_ID::DASH);
			}
			prevDashing_[id] = dashingNow;
		}
	}

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
			// 安全のためループSE停止
			SoundManager::GetInstance().StopSE(SE_ID::CHARGE_LOOP);
			SceneManager::GetInstance().ChangeScene(SceneManager::SCENE_ID::RESULT);
		}
	}

	// 10秒経過でボタンヒントフェードアウト
	if (showButtonHints_)
	{
		transitionTimer_++;
		if (transitionTimer_ > 600) // 約10秒
		{
			showButtonHints_ = false;
		}
	}
}

void GameScene::Draw()
{
	// ステージ
	Stage::GetInstance().Draw();

	// プレイヤー
	playerManager_->DrawPlayers();

	// マーカー描画
	DrawPlayerMarkers3D();

	// プレイヤー識別HUD
	//DrawPlayerHUDLegend();

	if(showButtonHints_)
	{
		// ボタン画像
		DrawGraph(20, 400, attackButtonImg_, TRUE);
		// + 文字
		DrawFormatString(80, 410, GetColor(255, 255, 255), "\n押してチャージ / 離して突進");

		DrawGraph(20, 500, moveButtonImg_, TRUE);
		// + 文字
		DrawFormatString(80, 510, GetColor(255, 255, 255), "\n移動 / 突進方向");
	}

}

void GameScene::DrawPlayerMarkers3D()
{
	const auto raw = playerManager_->GetPlayerRawPlayers();

	for (auto* p : raw)
	{
		if (!p || !p->IsAlive()) continue;

		const int playerID = p->GetID();
		const bool isHuman = (singlePlayerMode_ && playerID == 0);

		// ラベル生成
		char label[32];
		if (singlePlayerMode_)
		{
			snprintf(label, sizeof(label), "%s", isHuman ? "YOU" : "CPU");
		}
		else
		{
			snprintf(label, sizeof(label), "P%d", playerID + 1);
		}

		// 色設定
		unsigned int labelColor;
		unsigned int markerColor;
		if (singlePlayerMode_)
		{
			// 一人プレイ: 人間は赤、AIは灰色
			labelColor = isHuman ? GetColor(255, 60, 60) : GetColor(180, 180, 180);
			markerColor = isHuman ? GetColor(255, 60, 60) : GetColor(180, 180, 180);
		}
		else
		{
			// マルチプレイ: 1P赤, 2P青, 3P黄, 4P緑
			switch (playerID)
			{
			case 0: // 1P
				labelColor = GetColor(255, 60, 60);
				markerColor = GetColor(255, 60, 60);
				break;
			case 1: // 2P
				labelColor = GetColor(60, 120, 255);
				markerColor = GetColor(60, 120, 255);
				break;
			case 2: // 3P
				labelColor = GetColor(255, 220, 60);
				markerColor = GetColor(255, 220, 60);
				break;
			case 3: // 4P
				labelColor = GetColor(60, 220, 120);
				markerColor = GetColor(60, 220, 120);
				break;
			default:
				labelColor = GetColor(220, 220, 220);
				markerColor = GetColor(220, 220, 220);
				break;
			}
		}

		VECTOR playerPos = p->GetPos();
		VECTOR markerPos = VGet(playerPos.x, playerPos.y + 200.0f, playerPos.z);

		// 三角形（頭上の目印）を描画
		DrawTriangle3D(
			VGet(markerPos.x - 20.0f, markerPos.y + 20.0f, markerPos.z),
			VGet(markerPos.x + 20.0f, markerPos.y + 20.0f, markerPos.z),
			VGet(markerPos.x, markerPos.y, markerPos.z),
			markerColor,
			TRUE);

		// ラベルを重ねて表示
		DrawLabelAtWorld(label, VGet(markerPos.x, markerPos.y + 50.0f, markerPos.z), labelColor);
	}
}

void GameScene::DrawPlayerHUDLegend()
{
	unsigned int col = (youMarkerTimer_ < YOU_MARKER_HIGHLIGHT_FRAMES) ? GetColor(255, 255, 120) : GetColor(200, 220, 255);
	DrawFormatString(20, 60, col, "YOU: Player %d", humanPlayerId_ + 1);

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

	DeleteGraph(attackButtonImg_);
	DeleteGraph(moveButtonImg_);
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