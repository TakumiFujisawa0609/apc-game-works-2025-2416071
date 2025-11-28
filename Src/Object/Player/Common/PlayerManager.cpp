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

// 静的メンバ変数の定義
PlayerManager* PlayerManager::instance_ = nullptr;

// --- コンストラクタ / デストラクタ ---
PlayerManager::PlayerManager() {}

PlayerManager::~PlayerManager() {}

// --- インスタンス管理 ---

void PlayerManager::CreateInstance() {

	if (instance_ == nullptr) instance_ = new PlayerManager();
	instance_->Init();
}

PlayerManager& PlayerManager::GetInstance()
{
	if (instance_ == nullptr) PlayerManager::CreateInstance();
	return *instance_;
}

// --- 初期化 / 生成 / 解放 ---

void PlayerManager::Init()
{
	// プレイヤー配列クリア
	players_.clear();

	// Playerの死亡順序カウンタとManager側の状態をリセット
	Player::ResetDeathCounter();

	isGameOver_ = false;
	winnerID_ = -1;

	Reset();
}

void PlayerManager::InitAllPlayers()
{
	// 全プレイヤー初期化（リスポーン時など）
	for (auto& player : players_)
	{
		player->Init();
	}

	// 弾を全削除
	//AttackManager::GetInstance().Init();
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
	// 同じIDのプレイヤーが既に存在する場合は生成しない
	for (auto& player : players_) {
		if (player->GetID() == id) return;
	}

	// 新しいプレイヤーを生成
	std::shared_ptr<Player> newPlayer = nullptr;
	std::unique_ptr<InputController> controller = nullptr;

	// 入力設定 (ID1~4:移動 + ID5:ジャンプ + ID6:攻撃)
	KeyConfig keyConfig;
	InputManager::JOYPAD_NO padNo = static_cast<InputManager::JOYPAD_NO>(0);

	if (id == 0) {
		keyConfig = { KEY_INPUT_W, KEY_INPUT_S, KEY_INPUT_A, KEY_INPUT_D, KEY_INPUT_SPACE,KEY_INPUT_F };
		
		// PAD1を割り当て
		padNo = InputManager::JOYPAD_NO::PAD1;
	}
	else if (id == 1) {
		keyConfig = { KEY_INPUT_UP, KEY_INPUT_DOWN, KEY_INPUT_LEFT, KEY_INPUT_RIGHT, KEY_INPUT_RETURN, KEY_INPUT_RSHIFT};
		
		// PAD2を割り当て
		padNo = InputManager::JOYPAD_NO::PAD2;
	}
	else if (id == 2) {

		// 3P目はIJKL + RSHIFT
		keyConfig = { KEY_INPUT_I, KEY_INPUT_K, KEY_INPUT_J, KEY_INPUT_L, KEY_INPUT_Q,	KEY_INPUT_U };
		// PAD3を割り当て
		padNo = InputManager::JOYPAD_NO::PAD3;
	}
	else if (id == 3){
		// 4P目はテンキー + RCTRL
		keyConfig = { KEY_INPUT_NUMPAD8, KEY_INPUT_NUMPAD5, KEY_INPUT_NUMPAD4, KEY_INPUT_NUMPAD6, KEY_INPUT_E,KEY_INPUT_NUMPAD7 };
		// PAD4を割り当て
		padNo = InputManager::JOYPAD_NO::PAD4;
	}
	else {
		return;
	}

	// コントローラー生成
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
	// S全削除
	players_.clear();
	// e全削除
	BulletManager::GetInstance().Release();
}


void PlayerManager::UpdatePlayers(Stage& stage)
{

	// ゲームオーバーなら更新処理を停止
	if (isGameOver_) return;


	// ステージの傾きを更新
	stage.UpdateTilt(GetPlayerRawPlayers());

	// 全プレイヤー更新
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

	//死亡時はステージの傾き影響を受けないようにする
	stage.UpdateTilt(alive);

	// 弾との当たり判定
	CheckBulletCollisions();

	// プレイヤー同士の当たり判定
	CheckPlayerCollisions();

	// 勝敗判定
	CheckGameResult();

}

void PlayerManager::DrawPlayers()
{
	// デバッグ
	// プレイヤー同士が当たっているかいないか
	for (size_t i = 0; i < players_.size(); i++)
	{
		for (size_t j = i + 1; j < players_.size(); ++j)
		{
			Player* p1 = players_[i].get();
			Player* p2 = players_[j].get();
			// 距離計算
			VECTOR diff = VSub(p1->GetPos(), p2->GetPos());
			float distSq = diff.x * diff.x + diff.y * diff.y + diff.z * diff.z;
			float radSum = p1->GetCollisionRadius() + p2->GetCollisionRadius();
			// 衝突しているかチェック
			if (distSq < radSum * radSum)
			{
				// 衝突しているときは赤線で結ぶ
				DrawLine3D(p1->GetPos(), p2->GetPos(), GetColor(255, 0, 0));
				// 文字でも表示
				VECTOR midPos = VScale(VAdd(p1->GetPos(), p2->GetPos()), 0.5f);
				//DrawFormatString(midPos.x, midPos.y, GetColor(255, 0, 0), "当たっている");
			}
			else
			{
				// 衝突していないときは緑線で結ぶ
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
	// 既に決着がついていたら何もしない
	if (isGameOver_) return;

	// 2人以上いるかチェック（最低限のガード）
	if (players_.size() < 2) return;

	// 生存中のプレイヤーリストを作成
	std::vector<Player*> alivePlayers;
	for (const auto& player : players_) {
		if (player->IsAlive()) {
			alivePlayers.push_back(player.get());
		}
	}

	// 1. 1人だけ生存している場合 -> そのプレイヤーが勝者
	if (alivePlayers.size() == 1) {
		winnerID_ = alivePlayers[0]->GetID();
		isGameOver_ = true;
		return;
	}

	// 2. 全員死亡している場合 (alivePlayers.size() == 0)
	if (alivePlayers.empty()) {

		// 死亡順序でソートする (deathOrderが小さい=早く死んだ方が前)
		std::vector<Player*> sortedDeadPlayers = GetPlayerRawPlayers();

		// 死亡順序でソート (0は未死亡なので最後に回す)
		std::sort(sortedDeadPlayers.begin(), sortedDeadPlayers.end(), [](const Player* a, const Player* b) {
			// 死亡順序が0(未死亡)は後回し、それ以外は小さい順
			if (a->GetDeathOrder() == 0) return false;
			if (b->GetDeathOrder() == 0) return true;
			return a->GetDeathOrder() < b->GetDeathOrder();
			});

		// 死亡順序が最も遅い (ソートの最後尾) プレイヤーが勝者 (最後に落ちた)
		winnerID_ = sortedDeadPlayers.back()->GetID();
		isGameOver_ = true;
		return;
	}
}

std::vector<int> PlayerManager::GetPlayerRanks() const
{
	// プレイヤーを死亡順序でソート
	std::vector<Player*> sortedPlayers = GetPlayerRawPlayers();
	std::sort(sortedPlayers.begin(), sortedPlayers.end(), [](const Player* a, const Player* b) {
		// 死亡順序が0(未死亡)は後回し、それ以外は小さい順
		if (a->GetDeathOrder() == 0) return false;
		if (b->GetDeathOrder() == 0) return true;
		return a->GetDeathOrder() < b->GetDeathOrder();
		});
	// ソートされた順にIDを取得
	std::vector<int> ranks;
	for (const auto& player : sortedPlayers) {
		ranks.push_back(player->GetID());
	}
	return ranks;
}

// 生ポインタの配列を取得
std::vector<Player*> PlayerManager::GetPlayerRawPlayers() const
{
	std::vector<Player*> rawPlayers;
	for (auto& p : players_) rawPlayers.push_back(p.get());
	return rawPlayers;
}


// プレイヤー同士の当たり判定
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

				// ノックバック方向
				VECTOR knockBackDir = (dist > 0.0f) ? VScale(diff, 1.0f / dist) : VGet(1.0f, 0.0f, 0.0f);

				// ノックバック強さ（定数で調整可能）
				constexpr float KNOCKBACK_FORCE = 10.0f;

				VECTOR knockBack1 = VScale(knockBackDir, overlap * KNOCKBACK_FORCE);
				VECTOR knockBack2 = VScale(knockBackDir, -overlap * KNOCKBACK_FORCE);

				// プレイヤーにノックバックを適用
				p1->ApplyHit(knockBack1);
				p2->ApplyHit(knockBack2);
			}
		}
	}
}


void PlayerManager::CheckBulletCollisions()
{
	// 生ポインタ配列取得
	const auto& rawPlayers = GetPlayerRawPlayers();

	// 弾のすべての情報を取得
	const auto& bullets = BulletManager::GetInstance().GetBullets();

	// 各弾の当たり判定を確認
	for (const auto& b : bullets)
	{
		// 死んでいる弾は処理しない
		if (!b->IsAlive()) continue;

		// 弾の座標を取得
		const VECTOR& bPos = b->GetPos();

		// 当たり半径を取得
		float bRad = b->GetCollRad();

		// 各プレイヤーと当たり判定を確認
		for (Player* p : rawPlayers)
		{
			// 死んでいるプレイヤーは処理しない
			if (!p->IsAlive()) continue;

			// 弾の所有者IDを取得
			int ownerId = b->GetOwnerId();

			// 弾の所有者は無視
			if (p->GetID() == ownerId) continue;

			// プレイヤーとの距離判定
			VECTOR diff = VSub(bPos, p->GetPos());
			float distSq = diff.x * diff.x + diff.y * diff.y + diff.z * diff.z;
			float radSum = bRad + p->GetCollisionRadius();

			// 衝突しているかチェック
			if (distSq < radSum * radSum)
			{

				// 衝突しているときの処理
				// ノックバックベクトル計算
				VECTOR playerPos = p->GetPos();
				VECTOR dir = VSub(playerPos, bPos);
				dir.y = 0.0f; // 水平方向のみ
				float len = VSize(dir);
				VECTOR knockBackDir{};
				knockBackDir = (len > 0.0f) ? VScale(dir, 1.0f / len) : VGet(1.0f, 0.0f, 0.0f);
				VECTOR knockBack{};
				knockBack = VScale(knockBackDir, -KNOCKBACK_FORCE - 120000.0f);
				knockBack.y += KNOCKBACK_UPWARD_FORCE + 10.0f; // 上方向成分追加

				// プレイヤーにノックバックを適用
				p->ApplyHit(knockBack);

				// 弾を消す
				b->Kill();
				// 1つの弾が複数のプレイヤーに当たることはないのでループを抜ける
				break;
			}
		}
	}
}


void PlayerManager::Reset()
{
   ClearPlayers();
   // 各プレイヤーの解放
   for (auto& player : players_)
   {
	   player->Init();
   }
}

void PlayerManager::Release()
{
	if (instance_ != nullptr) 
	{
		// 先にプレイヤーたちの Release を呼ぶ
		for (auto& player : players_)
		{
			player->Release();
		}
		delete instance_;
		instance_ = nullptr;
	}
}



