#pragma once
#include <vector>
#include <memory>
#include "Player.h"
#include "../../Stage/Stage.h"
#include "../Control/InputController.h"

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
	// ノックバックの調整（水平方向・上方向）
	static constexpr float KNOCKBACK_FORCE = 3000.0f;        // 弾が当たった相手への水平ノックバック
	static constexpr float KNOCKBACK_UPWARD_FORCE = 200.0f;  // 弾ヒットの上向き成分

	// インスタンス生成
	static void CreateInstance();

	// インスタンス取得
	static PlayerManager& GetInstance();

	// 初期化
	void Init();

	// 全プレイヤー初期化
	void InitAllPlayers();

	// プレイヤー生成
	// コントローラを内部で生成する版
	void CreatePlayer(PlayerType type, int id, const PlayerParam& param);
	// AI/外部用に InputController を受け取る版
	void CreatePlayer(PlayerType type, int id, const PlayerParam& param, std::unique_ptr<InputController> controller);

	// 全プレイヤー更新
	void UpdatePlayers(Stage& stage);

	// 全プレイヤー描画
	void DrawPlayers();

	// 生のプレイヤーポインタ配列取得
	std::vector<Player*> GetPlayerRawPlayers() const;

	// 全削除
	void ClearPlayers();

	// 勝敗判定
	void CheckGameResult();

	// プレイヤーの順位を取得（死亡順に基づく）
	std::vector<int> GetPlayerRanks() const;

	// ゲームオーバー状態取得
	bool IsGameOver() const { return isGameOver_; }

	// 勝者ID取得
	int GetWinnerID() const { return winnerID_; }

	// ゲームオーバーフラグ取得（IsGameOverと同じ）
	bool GetIsGameOver() const { return isGameOver_; }

	// ゲーム開始時刻(ms)取得（弾回復開始の判定用）
	int GetGameStartTimeMs() const { return gameStartTimeMs_; }

	// プレイヤー同士の衝突判定
	void CheckPlayerCollisions();

	// 弾とプレイヤーの衝突判定
	void CheckBulletCollisions();

	// リセット
	void Reset();

	// 破棄
	void Release();

private:
	PlayerManager();
	~PlayerManager();

	// インスタンス
	static PlayerManager* instance_;
	std::vector<std::shared_ptr<Player>> players_;

	// ゲームオーバーフラグ
	bool isGameOver_ = false;

	// 勝者ID (-1: 未決, 0, 1...: プレイヤーID)
	int winnerID_ = -1;

	Player* player_ = nullptr;

	// 追加: ゲーム開始時刻（ms）
	int gameStartTimeMs_ = 0;
};