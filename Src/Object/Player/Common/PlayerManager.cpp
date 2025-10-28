#include "PlayerManager.h"
#include "Player.h"
#include "../Player/Player_1.h" 
#include "../Player/Player_2.h"
#include "../Player/Player_3.h"
#include "../Player/Player_4.h"
#include "../Control/InputController.h"
#include "../Control/Controller.h"
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
}

void PlayerManager::InitAllPlayers()
{
	// 全プレイヤー初期化（リスポーン時など）
	for (auto& player : players_)
	{
		player->Init();
	}
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

	// 入力設定 (ID 0: WASD + SPACE, ID 1: 矢印キー + ENTER)
	KeyConfig keyConfig;
	InputManager::JOYPAD_NO padNo = static_cast<InputManager::JOYPAD_NO>(0);

	if (id == 0) {
		keyConfig = { KEY_INPUT_W, KEY_INPUT_S, KEY_INPUT_A, KEY_INPUT_D, KEY_INPUT_SPACE };
		
		// PAD1を割り当て
		padNo = InputManager::JOYPAD_NO::PAD1;
	}
	else if (id == 1) {
		keyConfig = { KEY_INPUT_UP, KEY_INPUT_DOWN, KEY_INPUT_LEFT, KEY_INPUT_RIGHT, KEY_INPUT_RETURN };
		
		// PAD2を割り当て
		padNo = InputManager::JOYPAD_NO::PAD2;
	}
	else if (id == 2) {

		// 3P目はIJKL + RSHIFT
		keyConfig = { KEY_INPUT_I, KEY_INPUT_K, KEY_INPUT_J, KEY_INPUT_L, KEY_INPUT_RSHIFT };
		// PAD3を割り当て
		padNo = InputManager::JOYPAD_NO::PAD3;
	}
	else if (id == 3){
		// 4P目はテンキー + RCTRL
		keyConfig = { KEY_INPUT_NUMPAD8, KEY_INPUT_NUMPAD5, KEY_INPUT_NUMPAD4, KEY_INPUT_NUMPAD6, KEY_INPUT_RCONTROL };
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
		newPlayer = std::make_shared<Player_1>(id, param, std::move(controller));
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
	// 全削除
	players_.clear();
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

	// プレイヤー同士の当たり判定
	CheckPlayerCollisions();

	CheckGameResult();
}

void PlayerManager::DrawPlayers()
{

	// ゲームオーバー時のリザルト表示
	if (isGameOver_) {
		int color = GetColor(130, 130, 255); // 黄色

		DrawFormatString(300, 200, color, "===================");

		if (winnerID_ != -1) {
			// 勝者がいる場合
			DrawFormatString(300, 240, color, "WINNER IS PLAYER %d!", winnerID_ + 1);
			// 操作説明(点滅)
			int time = GetNowCount() / 500;
			if (time % 2 == 0)
				DrawFormatString2(300, 260, color, -1, "ENTERでタイトルに戻ります");
		}
		else {
			// 例外的な決着
			DrawFormatString(300, 240, color, "GAME OVER! DRAW!");
		}
		DrawFormatString(300, 280, color, "===================");
	}

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
	// 生ポインタ配列取得
	const auto& rawPlayers = GetPlayerRawPlayers();

	for (size_t i = 0; i < rawPlayers.size(); i++)
	{
		for (size_t j = i + 1; j < rawPlayers.size(); ++j)
		{
			Player* p1 = rawPlayers[i];
			Player* p2 = rawPlayers[j];

			// 生存している場合にのみ当たり判定を実施
			if (!p1->IsAlive() || !p2->IsAlive()) continue;

			// 当たり判定
			// 距離計算
			VECTOR diff = VSub(p1->GetPos(), p2->GetPos());
			float distSq = diff.x * diff.x + diff.y * diff.y + diff.z * diff.z;
			float radSum = p1->GetCollisionRadius() + p2->GetCollisionRadius();

			// 衝突しているかチェック
			if (distSq < radSum * radSum)
			{
				// 衝突しているときはお互いを押し返す
				float dist = sqrtf(distSq);

				// 押し出し方向(0除算控除として　dist > 0.0f)
				VECTOR pushDir = (dist > 0.0f) ? VScale(diff, 1.0f / dist) : VGet(1.0f, 0.0f, 0.0f);

				// めり込み量
				float overlap = radSum - dist;

				// 重さを使って押し出す値方を計算する
				float w1 = p1->GetWeight();
				float w2 = p2->GetWeight();
				float inMv1 = 1.0f / w1;
				float inMv2 = 1.0f / w2;
				float totalInvMass = inMv1 + inMv2;

				// 押し出しベクトル計算
				VECTOR pushDirFlat = pushDir;
				pushDirFlat.y = 0.0f;
				VECTOR pVec1 = VScale(pushDirFlat, overlap * (inMv1 / totalInvMass));
				VECTOR pVec2 = VScale(pushDirFlat, overlap * (inMv2 / totalInvMass));

				// 位置更新
				p1->SetPos(VAdd(p1->GetPos(), pVec1));
				p2->SetPos(VSub(p2->GetPos(), pVec2));

			}
		}
	}
}

void PlayerManager::Reset()
{
	ClearPlayers();
	Init();
}

void PlayerManager::AddAttackObject(AttackObj* attackObj)  
{  
   // std::unique_ptrに変換してからpush_backする  
   attackObjects_.push_back(std::unique_ptr<AttackObj>(attackObj));  
}
