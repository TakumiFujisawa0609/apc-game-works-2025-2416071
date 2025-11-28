#pragma once
#include <vector>
#include <memory>
#include "Player.h"
#include "../../Stage/Stage.h"
#include "../Control/InputController.h"

// Playerの識別
enum class PlayerType
{
	Player_1,
	Player_2,
	Player_3,
	Player_4
};

class PlayerManager
{
public:

	// 定数
	static constexpr float KNOCKBACK_FORCE = 20.0f; // プレイヤー同士が衝突したときのノックバック力
	static constexpr float KNOCKBACK_UPWARD_FORCE = 5.0f; // ノックバックの上方向成分


	// インスタンス生成
	static void CreateInstance();

	// インスタンス取得
	static PlayerManager& GetInstance();

	// 初期化
	void Init();

	// 全プレイヤー初期化
	void InitAllPlayers();


	// プレイヤー生成
	// 既存の「内部で Controller を生成する」CreatePlayer
	void CreatePlayer(PlayerType type, int id, const PlayerParam& param);
	// AI 用の InputController を渡せるようにする。
	void CreatePlayer(PlayerType type, int id, const PlayerParam& param, std::unique_ptr<InputController> controller);

	// 全プレイヤー更新
	void UpdatePlayers(Stage& stage);

	// 全プレイヤー描画
	void DrawPlayers();

	// 生データのプレイヤー配列を取得
	std::vector<Player*> GetPlayerRawPlayers() const;

	// 全削除
	void ClearPlayers();

	// 勝敗判定を行う
	void CheckGameResult();

	// プレイヤーの順位を取得
	std::vector<int> GetPlayerRanks() const;

	

	// ゲームオーバー状態を取得
	bool IsGameOver() const { return isGameOver_; }

	// 勝者IDを取得
	int GetWinnerID() const { return winnerID_; }

	// ゲームオーバーフラグを取得
	bool GetIsGameOver() const { return isGameOver_; }

	// プレイヤー同士の衝突判定
	void CheckPlayerCollisions();

	// 弾との衝突判定
	void CheckBulletCollisions();

	// リセット
	void Reset();

	// 解放
	void Release();

private:

	PlayerManager();
	~PlayerManager();

	// インスタンス
	static PlayerManager* instance_;
	std::vector<std::shared_ptr<Player>> players_;

	// ゲーム終了フラグ (勝者が決定したらtrue)
	bool isGameOver_ = false;

	// 勝者ID (-1: 未決定, 0, 1...: プレイヤーID)
	int winnerID_ = -1;

	Player* player_;

};