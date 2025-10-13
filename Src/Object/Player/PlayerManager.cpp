#include "PlayerManager.h"
#include "Player.h"
#include <DxLib.h>
#include "Player_1.h"
#include "Player_2.h"
#include <memory> 
#include "Control/InputController.h" // KeyConfigのために必要
#include "Control/KeyController.h"   // KeyControllerを使うために必要

// 静的メンバ変数の定義
PlayerManager* PlayerManager::instance_ = nullptr;

// コンストラクタ
PlayerManager::PlayerManager() {}

// デストラクタ
PlayerManager::~PlayerManager() {}

// インスタンス生成
void PlayerManager::CreateInstance() {

	if (instance_ == nullptr) instance_ = new PlayerManager();
	instance_->Init();
}

// インスタンス取得
PlayerManager& PlayerManager::GetInstance()
{
	if (instance_ == nullptr) PlayerManager::CreateInstance();
	return *instance_;
}

// 初期化
void PlayerManager::Init()
{
	// プレイヤー配列クリア
	players_.clear();
}

// 全プレイヤー初期化
void PlayerManager::InitAllPlayers()
{
	for (auto& player : players_)
	{
		player->Init();
	}
}

// プレイヤー生成
// ★ 修正点1: inputIdを削除（コンストラクタで直接渡さなくなったため）
void PlayerManager::CreatePlayer(PlayerType type, int id, float weight)
{
	// 重複防止
	for (auto& player : players_) {
		if (player->GetID() == id) return;
	}

	std::shared_ptr<Player> newPlayer = nullptr;
	std::unique_ptr<InputController> controller = nullptr; // InputControllerの準備
	KeyConfig keyConfig; // キー設定の準備

	// IDに基づいてキー設定データを作成
	if (id == 0) {
		// P1設定: WASD + SPACE
		keyConfig = { KEY_INPUT_W, KEY_INPUT_S, KEY_INPUT_A, KEY_INPUT_D, KEY_INPUT_SPACE };
	}
	else if (id == 1) {
		// P2設定: 矢印キー + RETURN
		keyConfig = { KEY_INPUT_UP, KEY_INPUT_DOWN, KEY_INPUT_LEFT, KEY_INPUT_RIGHT, KEY_INPUT_RETURN };
	}
	else {
		return;
	}

	// KeyControllerに設定データを渡して生成
	controller = std::make_unique<KeyController>(keyConfig);

	switch (type)
	{
	case PlayerType::Player_1:
		newPlayer = std::make_shared<Player_1>(id, weight, std::move(controller));
		break;
	case PlayerType::Player_2:
		newPlayer = std::make_shared<Player_2>(id, weight, std::move(controller));
		break;
	default:
		return;
	}

	players_.push_back(newPlayer);
}

// 全プレイヤー更新
void PlayerManager::UpdatePlayers(Stage& stage)
{
	// ステージの傾きを更新
	stage.UpdateTilt(GetPlayerRawPlayers());

	// 全プレイヤー更新
	for (auto& player : players_)
	{
		player->Update();
	}
}

// 全プレイヤー描画
void PlayerManager::DrawPlayers()
{
	for (auto& player : players_)
	{
		player->Draw();
	}
}

// 全プレイヤー解放
void PlayerManager::ClearPlayers()
{
	players_.clear();
}

// 生ポインタの配列を取得
std::vector<Player*> PlayerManager::GetPlayerRawPlayers() const
{
	std::vector<Player*> rawPlayers;
	for (auto& p : players_) rawPlayers.push_back(p.get());
	return rawPlayers;
}