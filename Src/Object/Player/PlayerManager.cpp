#include "PlayerManager.h"
#include "Player.h"
#include <DxLib.h>
#include "Player_1.h"

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
void PlayerManager::CreatePlayer(PlayerType type ,int id, float weight)
{
	// 重複防止
	for (auto& player : players_) {
		if (player->GetID() == id) return;
	}

	std::shared_ptr<Player> newPlayer = nullptr;

	switch (type)
	{
	case PlayerType::Player_1:
		newPlayer = std::make_shared<Player_1>(id, weight);
		break;
		// 他のプレイヤータイプもここに追加可能
	default:
		return; // 未知のタイプの場合は何もしない
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


