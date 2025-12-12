#include "PlayerManager.h"
#include "Player.h"
#include "../Player/Player_1.h" 
#include "../Player/Player_2.h"
#include "../Player/Player_3.h"
#include "../Player/Player_4.h"
#include "../Control/InputController.h"
#include "../Control/Controller.h"
#include "../../../Utility/AsoUtility.h"
#include "../../Bullet/BulletManager.h"
#include <DxLib.h>
#include <memory>
#include <algorithm>

// 静的インスタンス
PlayerManager* PlayerManager::instance_ = nullptr;

// コンストラクタ / デストラクタ
PlayerManager::PlayerManager() {}
PlayerManager::~PlayerManager() {}

// インスタンス管理
void PlayerManager::CreateInstance() {
	if (instance_ == nullptr) instance_ = new PlayerManager();
	instance_->Init();
}

PlayerManager& PlayerManager::GetInstance()
{
	if (instance_ == nullptr) PlayerManager::CreateInstance();
	return *instance_;
}

// 初期化 / リセット / 解放
void PlayerManager::Init()
{
	// プレイヤー配列クリア
	players_.clear();

	// Playerの死亡順カウンタ初期化
	Player::ResetDeathCounter();

	isGameOver_ = false;
	winnerID_ = -1;

	// ゲーム開始時刻記録（弾回復開始30秒の判定用）
	gameStartTimeMs_ = GetNowCount();

	Reset();
}

void PlayerManager::InitAllPlayers()
{
	// 全プレイヤー初期化（リスポーンなど）
	for (auto& player : players_)
	{
		player->Init();
	}
	// 弾などのクリアが必要ならここで
	// BulletManager::GetInstance().Init(); // 必要なら用意
}

void PlayerManager::CreatePlayer(PlayerType type, int id, const PlayerParam& param, std::unique_ptr<InputController> controller)
{
	// ID 重複チェック
	for (auto& player : players_) {
		if (player->GetID() == id) return;
	}

	if (!controller) return;

	std::shared_ptr<Player> newPlayer = nullptr;

	switch (type)
	{
	case PlayerType::Player_1:
		newPlayer = std::make_shared<Player_1>(id, param, std::move(controller));
		break;
	case PlayerType::Player_2:
		newPlayer = std::make_shared<Player_2>(id, param, std::move(controller));
		break;
	case PlayerType::Player_3:
		newPlayer = std::make_shared<Player_3>(id, param, std::move(controller));
		break;
	case PlayerType::Player_4:
		newPlayer = std::make_shared<Player_4>(id, param, std::move(controller));
		break;
	default:
		break;
	}

	if (newPlayer) players_.push_back(newPlayer);
}

void PlayerManager::CreatePlayer(PlayerType type, int id, const PlayerParam& param)
{
	// 既に同一IDのプレイヤーが存在する場合は生成しない
	for (auto& player : players_) {
		if (player->GetID() == id) return;
	}

	// 新しいプレイヤーを生成
	std::shared_ptr<Player> newPlayer = nullptr;
	std::unique_ptr<InputController> controller = nullptr;

	// 入力設定 （ID0~3: キーボード＋ゲームパッド）
	KeyConfig keyConfig;
	InputManager::JOYPAD_NO padNo = static_cast<InputManager::JOYPAD_NO>(0);

	if (id == 0) {
		keyConfig = { KEY_INPUT_W, KEY_INPUT_S, KEY_INPUT_A, KEY_INPUT_D, KEY_INPUT_SPACE, KEY_INPUT_F };
		padNo = InputManager::JOYPAD_NO::PAD1;
	}
	else if (id == 1) {
		keyConfig = { KEY_INPUT_UP, KEY_INPUT_DOWN, KEY_INPUT_LEFT, KEY_INPUT_RIGHT, KEY_INPUT_RETURN, KEY_INPUT_RSHIFT };
		padNo = InputManager::JOYPAD_NO::PAD2;
	}
	else if (id == 2) {
		keyConfig = { KEY_INPUT_I, KEY_INPUT_K, KEY_INPUT_J, KEY_INPUT_L, KEY_INPUT_Q, KEY_INPUT_U };
		padNo = InputManager::JOYPAD_NO::PAD3;
	}
	else if (id == 3) {
		keyConfig = { KEY_INPUT_NUMPAD8, KEY_INPUT_NUMPAD5, KEY_INPUT_NUMPAD4, KEY_INPUT_NUMPAD6, KEY_INPUT_E, KEY_INPUT_NUMPAD7 };
		padNo = InputManager::JOYPAD_NO::PAD4;
	}
	else {
		return;
	}

	// コントローラーを生成
	controller = std::make_unique<Controller>(keyConfig, padNo);

	switch (type)
	{
	case PlayerType::Player_1:
		newPlayer = std::make_shared<Player_1>(id, param, std::move(controller));
		break;
	case PlayerType::Player_2:
		newPlayer = std::make_shared<Player_2>(id, param, std::move(controller));
		break;
	case PlayerType::Player_3:
		newPlayer = std::make_shared<Player_3>(id, param, std::move(controller));
		break;
	case PlayerType::Player_4:
		newPlayer = std::make_shared<Player_4>(id, param, std::move(controller));
		break;
	default:
		break;
	}

	players_.push_back(newPlayer);
}

void PlayerManager::ClearPlayers()
{
	// 各 Player::Release() を呼ぶ
	for (auto& player : players_) {
		player->Release();
	}
	// プレイヤー配列をクリア
	players_.clear();

	// 弾のクリア
	BulletManager::GetInstance().Release();
}

void PlayerManager::UpdatePlayers(Stage& stage)
{
	// ゲームオーバーなら更新停止
	if (isGameOver_) return;

	// ステージ傾き（全プレイヤー）更新
	stage.UpdateTilt(GetPlayerRawPlayers());

	// 各プレイヤー更新
	for (auto& player : players_)
	{
		player->Update();
	}

	// 生存プレイヤー配列
	std::vector<Player*> alive;
	alive.reserve(players_.size());

	for (auto& p : players_)
	{
		if (p->IsAlive())
		{
			alive.push_back(p.get());
		}
	}

	// 生存者のみでステージ傾き再更新（落下中を除外したい場合の二段階更新）
	stage.UpdateTilt(alive);

	// 弾の衝突
	CheckBulletCollisions();

	// プレイヤー同士の衝突
	CheckPlayerCollisions();

	// 勝敗判定
	CheckGameResult();
}

void PlayerManager::DrawPlayers()
{
	// デバッグ：プレイヤー間距離の可視化
	for (size_t i = 0; i < players_.size(); i++)
	{
		for (size_t j = i + 1; j < players_.size(); ++j)
		{
			Player* p1 = players_[i].get();
			Player* p2 = players_[j].get();
			VECTOR diff = VSub(p1->GetPos(), p2->GetPos());
			float distSq = diff.x * diff.x + diff.y * diff.y + diff.z * diff.z;
			float radSum = p1->GetCollisionRadius() + p2->GetCollisionRadius();
			if (distSq < radSum * radSum)
			{
				DrawLine3D(p1->GetPos(), p2->GetPos(), GetColor(255, 0, 0));
			}
			else
			{
				DrawLine3D(p1->GetPos(), p2->GetPos(), GetColor(0, 255, 0));
			}
		}
	}

	// 全プレイヤー描画
	for (auto& player : players_)
	{
		player->Draw();
	}
}

// 勝敗判定
void PlayerManager::CheckGameResult()
{
	// 既に決着済みなら何もしない
	if (isGameOver_) return;

	// 2人以上でなければガード
	if (players_.size() < 2) return;

	// 生存プレイヤーのリスト
	std::vector<Player*> alivePlayers;
	for (const auto& player : players_) {
		if (player->IsAlive()) {
			alivePlayers.push_back(player.get());
		}
	}

	// 1. 生存者が1名 -> そのプレイヤーが勝者
	if (alivePlayers.size() == 1) {
		winnerID_ = alivePlayers[0]->GetID();
		isGameOver_ = true;
		return;
	}

	// 2. 誰も生きていない場合 -> 最後に死んだプレイヤーが勝者
	if (alivePlayers.empty()) {

		// 全プレイヤーを死亡順でソート（deathOrder==0 は最初に除外扱い）
		std::vector<Player*> sortedDeadPlayers = GetPlayerRawPlayers();

		std::sort(sortedDeadPlayers.begin(), sortedDeadPlayers.end(), [](const Player* a, const Player* b) {
			if (a->GetDeathOrder() == 0) return false;
			if (b->GetDeathOrder() == 0) return true;
			return a->GetDeathOrder() < b->GetDeathOrder();
			});

		// 最後に死亡したプレイヤーを勝者に
		winnerID_ = sortedDeadPlayers.back()->GetID();
		isGameOver_ = true;
		return;
	}
}

std::vector<int> PlayerManager::GetPlayerRanks() const
{
	std::vector<Player*> sortedPlayers = GetPlayerRawPlayers();
	std::sort(sortedPlayers.begin(), sortedPlayers.end(), [](const Player* a, const Player* b) {
		if (a->GetDeathOrder() == 0) return false;
		if (b->GetDeathOrder() == 0) return true;
		return a->GetDeathOrder() < b->GetDeathOrder();
		});
	std::vector<int> ranks;
	for (const auto& player : sortedPlayers) {
		ranks.push_back(player->GetID());
	}
	return ranks;
}

// 生のポインタ配列
std::vector<Player*> PlayerManager::GetPlayerRawPlayers() const
{
	std::vector<Player*> rawPlayers;
	for (auto& p : players_) rawPlayers.push_back(p.get());
	return rawPlayers;
}

// プレイヤー同士の衝突
void PlayerManager::CheckPlayerCollisions()
{
	const auto& rawPlayers = GetPlayerRawPlayers();

	for (size_t i = 0; i < rawPlayers.size(); i++)
	{
		for (size_t j = i + 1; j < rawPlayers.size(); ++j)
		{
			Player* p1 = rawPlayers[i];
			Player* p2 = rawPlayers[j];

			if (!p1->IsAlive() || !p2->IsAlive()) continue;

			VECTOR diff = VSub(p1->GetPos(), p2->GetPos());
			float distSq = diff.x * diff.x + diff.y * diff.y + diff.z * diff.z;
			float radSum = p1->GetCollisionRadius() + p2->GetCollisionRadius();

			if (distSq < radSum * radSum)
			{
				float dist = sqrtf(distSq);
				float overlap = radSum - dist;

				VECTOR knockBackDir = (dist > 0.0f) ? VScale(diff, 1.0f / dist) : VGet(1.0f, 0.0f, 0.0f);

				constexpr float KNOCKBACK_FORCE_LOCAL = 10.0f;

				VECTOR knockBack1 = VScale(knockBackDir, overlap * KNOCKBACK_FORCE_LOCAL);
				VECTOR knockBack2 = VScale(knockBackDir, -overlap * KNOCKBACK_FORCE_LOCAL);

				p1->ApplyHit(knockBack1);
				p2->ApplyHit(knockBack2);
			}
		}
	}
}

// 弾の衝突
void PlayerManager::CheckBulletCollisions()
{
	const auto& rawPlayers = GetPlayerRawPlayers();
	const auto& bullets = BulletManager::GetInstance().GetBullets();

	for (const auto& b : bullets)
	{
		if (!b->IsAlive()) continue;

		const VECTOR& bPos = b->GetPos();
		float bRad = b->GetCollRad();

		for (Player* p : rawPlayers)
		{
			if (!p->IsAlive()) continue;

			int ownerId = b->GetOwnerId();
			if (p->GetID() == ownerId) continue;

			VECTOR diff = VSub(bPos, p->GetPos());
			float distSq = diff.x * diff.x + diff.y * diff.y + diff.z * diff.z;
			float radSum = bRad + p->GetCollisionRadius();

			if (distSq < radSum * radSum)
			{
				// ノックバック方向（水平正規化）
				VECTOR playerPos = p->GetPos();
				VECTOR dir = VSub(playerPos, bPos);
				dir.y = 0.0f;
				float len = VSize(dir);
				VECTOR knockBackDir = (len > 1e-5f) ? VScale(dir, 1.0f / len) : VGet(1.0f, 0.0f, 0.0f);

				VECTOR knockBack = VScale(knockBackDir, KNOCKBACK_FORCE);
				knockBack.y += KNOCKBACK_UPWARD_FORCE;

				p->ApplyHit(knockBack);

				// 弾は1ヒットで消える
				b->Kill();
				break;
			}
		}
	}
}

void PlayerManager::Reset()
{
	ClearPlayers();
	// 再初期化（必要ならここでプレイヤー再生成など）
	for (auto& player : players_)
	{
		player->Init();
	}
}

void PlayerManager::Release()
{
	if (instance_ != nullptr)
	{
		for (auto& player : players_)
		{
			player->Release();
		}
		delete instance_;
		instance_ = nullptr;
	}
}