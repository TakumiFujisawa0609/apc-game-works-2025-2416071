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
#include <cmath>

// 静的インスタンス
PlayerManager* PlayerManager::instance_ = nullptr;

// 追加: キャラクターのデフォルトパラメータ
PlayerParam PlayerManager::GetDefaultParamForType(PlayerType type)
{
	PlayerParam param;
	switch (type)
	{
	case PlayerType::Player_1: param.weight = 10.0f; param.speed = 4.0f; param.jumpPower = 7.0f; break;
	case PlayerType::Player_2: param.weight = 10.0f; param.speed = 4.0f; param.jumpPower = 5.0f; break;
	case PlayerType::Player_3: param.weight = 10.0f; param.speed = 4.0f; param.jumpPower = 2.0f; break;
	case PlayerType::Player_4: param.weight = 10.0f; param.speed = 4.0f; param.jumpPower = 2.0f; break;
	default:                   param.weight = 10.0f; param.speed = 4.0f; param.jumpPower = 5.0f; break;
	}
	return param;
}

// 追加: モデルパス取得
const char* PlayerManager::GetModelPathForType(PlayerType type)
{
	switch (type)
	{
	case PlayerType::Player_1: return "Data/Model/Player/Body_AA_01.mv1";
	case PlayerType::Player_2: return "Data/Model/Player/Body_AB_01.mv1";
	case PlayerType::Player_3: return "Data/Model/Player/Body_AC_01.mv1";
	case PlayerType::Player_4: return "Data/Model/Player/Body_AD_01.mv1";
	default: return "Data/Model/Player/Body_AA_01.mv1";
	}
}

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
	players_.clear();
	Player::ResetDeathCounter();

	isGameOver_ = false;
	winnerID_ = -1;

	// 弾回復開始の30秒判定用
	gameStartTimeMs_ = GetNowCount();

	Reset();
}

void PlayerManager::InitAllPlayers()
{
	for (auto& player : players_)
	{
		player->Init();
	}
}

void PlayerManager::CreatePlayer(PlayerType type, int id, const PlayerParam& param, std::unique_ptr<InputController> controller)
{
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
	for (auto& player : players_) {
		if (player->GetID() == id) return;
	}

	std::shared_ptr<Player> newPlayer = nullptr;
	std::unique_ptr<InputController> controller = nullptr;

	KeyConfig keyConfig;
	InputManager::JOYPAD_NO padNo = static_cast<InputManager::JOYPAD_NO>(0);

	if (id == 0) {
		// Player1: 移動=WASD, ジャンプ=Space, 攻撃=F, ため=LShift
		keyConfig = { KEY_INPUT_W, KEY_INPUT_S, KEY_INPUT_A, KEY_INPUT_D, KEY_INPUT_SPACE, KEY_INPUT_F, KEY_INPUT_LSHIFT };
		padNo = InputManager::JOYPAD_NO::PAD1;
	}
	else if (id == 1) {
		// Player2: 矢印＋Enter, 攻撃=右Shift, ため=左Ctrl
		keyConfig = { KEY_INPUT_UP, KEY_INPUT_DOWN, KEY_INPUT_LEFT, KEY_INPUT_RIGHT, KEY_INPUT_RETURN, KEY_INPUT_RSHIFT, KEY_INPUT_LCONTROL };
		padNo = InputManager::JOYPAD_NO::PAD2;
	}
	else if (id == 2) {
		// Player3: IJKL＋Q攻撃＋Oため
		keyConfig = { KEY_INPUT_I, KEY_INPUT_K, KEY_INPUT_J, KEY_INPUT_L, KEY_INPUT_Q, KEY_INPUT_U, KEY_INPUT_O };
		padNo = InputManager::JOYPAD_NO::PAD3;
	}
	else if (id == 3) {
		// Player4: テンキー移動＋Eジャンプ＋ためは仮でRShift（後から変更）
		keyConfig = { KEY_INPUT_NUMPAD8, KEY_INPUT_NUMPAD5, KEY_INPUT_NUMPAD4, KEY_INPUT_NUMPAD6, KEY_INPUT_E, KEY_INPUT_NUMPAD7, KEY_INPUT_RSHIFT };
		padNo = InputManager::JOYPAD_NO::PAD4;
	}
	else {
		return;
	}

	controller = std::make_unique<Controller>(keyConfig, padNo);

	switch (type)
	{
	case PlayerType::Player_1: newPlayer = std::make_shared<Player_1>(id, param, std::move(controller)); break;
	case PlayerType::Player_2: newPlayer = std::make_shared<Player_2>(id, param, std::move(controller)); break;
	case PlayerType::Player_3: newPlayer = std::make_shared<Player_3>(id, param, std::move(controller)); break;
	case PlayerType::Player_4: newPlayer = std::make_shared<Player_4>(id, param, std::move(controller)); break;
	default: break;
	}

	players_.push_back(newPlayer);
}

void PlayerManager::ClearPlayers()
{
	for (auto& player : players_) {
		player->Release();
	}
	players_.clear();

	BulletManager::GetInstance().Release();
}

void PlayerManager::UpdatePlayers(Stage& stage)
{
	if (isGameOver_) return;

	stage.UpdateTilt(GetPlayerRawPlayers());

	for (auto& player : players_)
	{
		player->Update();
	}

	std::vector<Player*> alive;
	alive.reserve(players_.size());
	for (auto& p : players_)
	{
		if (p->IsAlive())
		{
			alive.push_back(p.get());
		}
	}

	stage.UpdateTilt(alive);

	CheckBulletCollisions();
	CheckPlayerCollisions();

	CheckGameResult();
}

void PlayerManager::DrawPlayers()
{
	// デバッグ: プレイヤー間の距離表示ライン
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

	for (auto& player : players_)
	{
		player->Draw();
	}
}

// 勝敗判定
void PlayerManager::CheckGameResult()
{
	if (isGameOver_) return;
	if (players_.size() < 2) return;

	std::vector<Player*> alivePlayers;
	for (const auto& player : players_) {
		if (player->IsAlive()) {
			alivePlayers.push_back(player.get());
		}
	}

	if (alivePlayers.size() == 1) {
		winnerID_ = alivePlayers[0]->GetID();
		isGameOver_ = true;
		return;
	}

	if (alivePlayers.empty()) {
		std::vector<Player*> sortedDeadPlayers = GetPlayerRawPlayers();

		std::sort(sortedDeadPlayers.begin(), sortedDeadPlayers.end(), [](const Player* a, const Player* b) {
			if (a->GetDeathOrder() == 0) return false;
			if (b->GetDeathOrder() == 0) return true;
			return a->GetDeathOrder() < b->GetDeathOrder();
			});

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

std::vector<Player*> PlayerManager::GetPlayerRawPlayers() const
{
	std::vector<Player*> rawPlayers;
	for (auto& p : players_) rawPlayers.push_back(p.get());
	return rawPlayers;
}

// プレイヤー同士の衝突（突進ヒット対応　通常押し合い）
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

			const VECTOR pos1 = p1->GetPos();
			const VECTOR pos2 = p2->GetPos();

			VECTOR diffXZ = { pos1.x - pos2.x, 0.0f, pos1.z - pos2.z };
			float distXZ = VSize(diffXZ);
			float radSum = p1->GetCollisionRadius() + p2->GetCollisionRadius();

			if (distXZ >= radSum) continue;

			// どちらか一方がダッシュ中なら、攻撃側→防御側に片方向ノックバックを与える
			const bool d1 = p1->IsDashing();
			const bool d2 = p2->IsDashing();
			if (d1 ^ d2)
			{
				Player* attacker = d1 ? p1 : p2;
				Player* defender = d1 ? p2 : p1;

				// ダッシュ方向（水平）
				VECTOR kbDir = attacker->GetDashDir();
				kbDir.y = 0.0f;
				float kbl = VSize(kbDir);
				if (kbl < 1e-5f) {
					// 念のため、相手への方向にフォールバック
					kbDir = VSub(defender->GetPos(), attacker->GetPos());
					kbDir.y = 0.0f;
					kbl = VSize(kbDir);
				}
				if (kbl > 1e-5f) kbDir = VScale(kbDir, 1.0f / kbl);

				// ため時間に応じたインパクト強度を使用（水平）　上向き定数
				float power = attacker->GetDashImpactPower(); // プレイヤー側で設定済み（12万　40万程度）
				VECTOR knockBack = VScale(kbDir, power);
				knockBack.y += DASH_KNOCKBACK_UPWARD;

				defender->ApplyHit(knockBack);

				// 多重ヒット防止：攻撃側のダッシュを終了
				attacker->EndDashOnHit();

				// 片方向ノックバック完了。通常押し合いはスキップ
				continue;
			}

			// 双方ダッシュでない場合は通常押し合い（簡易カプセル：水平　縦距離係数）
			float dy = std::fabs(pos1.y - pos2.y);
			const float CAP_HEIGHT = 180.0f;
			float heightFactorTmp = 1.0f - (dy / CAP_HEIGHT);
			float heightFactor = (heightFactorTmp < 0.0f) ? 0.0f : heightFactorTmp;

			float overlap = (radSum - distXZ) * heightFactor;
			if (overlap <= 0.0f) continue;

			VECTOR dir = (distXZ > 1e-5f) ? VScale(diffXZ, 1.0f / distXZ) : VGet(1.0f, 0.0f, 0.0f);

			// 押し戻し（弱め）
			const float KNOCKBACK_FORCE_LOCAL = 6.0f;
			VECTOR knockBack1 = VScale(dir, overlap * KNOCKBACK_FORCE_LOCAL);
			VECTOR knockBack2 = VScale(dir, -overlap * KNOCKBACK_FORCE_LOCAL);

			p1->ApplyHit(knockBack1);
			p2->ApplyHit(knockBack2);
		}
	}
}

// 弾とプレイヤーの衝突（連続判定：Swept Sphere）
void PlayerManager::CheckBulletCollisions()
{
	const auto& rawPlayers = GetPlayerRawPlayers();
	const auto& bullets = BulletManager::GetInstance().GetBullets();

	for (const auto& b : bullets)
	{
		if (!b->IsAlive()) continue;

		const VECTOR& p0 = b->GetPrevPos();
		const VECTOR& p1 = b->GetPos();
		VECTOR v = VSub(p1, p0);
		float vv = VDot(v, v);

		// 1発につき最初に当たった対象のみ
		bool hitSomeone = false;
		float bestT = 1.0f;
		Player* bestTarget = nullptr;
		VECTOR bestClosest = p1;

		for (Player* p : rawPlayers)
		{
			if (!p->IsAlive()) continue;

			// 発射者本人は除外
			if (p->GetID() == b->GetOwnerId()) continue;

			// 合成半径
			float R = b->GetCollRad() + p->GetCollisionRadius();

			// 弾がほぼ停止：点と球
			if (vv < 1e-6f)
			{
				VECTOR d = VSub(p1, p->GetPos());
				float d2 = VDot(d, d);
				if (d2 <= R * R)
				{
					bestTarget = p;
					bestClosest = p1;
					bestT = 0.0f;
					hitSomeone = true;
				}
				continue;
			}

			// 線分p0->p1 と 球(c, R) の最近接点
			VECTOR c = p->GetPos();
			VECTOR w = VSub(c, p0);
			float t = VDot(w, v) / vv;
			if (t < 0.0f) t = 0.0f;
			else if (t > 1.0f) t = 1.0f;

			VECTOR closest = VAdd(p0, VScale(v, t));
			VECTOR d = VSub(closest, c);
			float dist2 = VDot(d, d);
			if (dist2 <= R * R)
			{
				if (t < bestT)
				{
					bestT = t;
					bestTarget = p;
					bestClosest = closest;
					hitSomeone = true;
				}
			}
		}

		if (hitSomeone && bestTarget)
		{
			// 衝突点→プレイヤーの水平正規化ベクトルでノックバック
			VECTOR kbDir = VSub(bestTarget->GetPos(), bestClosest);
			kbDir.y = 0.0f;
			float len = VSize(kbDir);
			kbDir = (len > 1e-5f) ? VScale(kbDir, 1.0f / len) : VGet(1.0f, 0.0f, 0.0f);

			VECTOR knockBack = VScale(kbDir, KNOCKBACK_FORCE);
			knockBack.y += KNOCKBACK_UPWARD_FORCE;

			bestTarget->ApplyHit(knockBack);

			// 弾は1ヒットで消える
			b->Kill();
		}
	}
}

void PlayerManager::Reset()
{
	ClearPlayers();
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