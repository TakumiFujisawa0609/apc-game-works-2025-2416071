#pragma once
#include <vector>
#include <memory>
#include "Player.h"
#include "../../Stage/Stage.h"
#include "../Control/InputController.h"

// プレイヤー種類
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
	// ノックバック強度（弾ヒット）
	static constexpr float KNOCKBACK_FORCE = 150000.0f; // 水平ノックバック（弾）
	static constexpr float KNOCKBACK_UPWARD_FORCE = 750.0f;    // 上向きノックバック（弾）

	// 突進ヒット専用の上向き成分
	static constexpr float DASH_KNOCKBACK_UPWARD = 900.0f;

	// 追加: キャラクターのデフォルトパラメータ・モデルパス取得
	static PlayerParam GetDefaultParamForType(PlayerType type);
	static const char* GetModelPathForType(PlayerType type);

	// インスタンス生成/取得
	static void CreateInstance();
	static PlayerManager& GetInstance();

	// 初期化
	void Init();

	// 全プレイヤー初期化
	void InitAllPlayers();

	// プレイヤー生成
	void CreatePlayer(PlayerType type, int id, const PlayerParam& param);
	void CreatePlayer(PlayerType type, int id, const PlayerParam& param, std::unique_ptr<InputController> controller);

	// 更新/描画
	void UpdatePlayers(Stage& stage);
	void DrawPlayers();

	// プレイヤー配列（生ポインタ）
	std::vector<Player*> GetPlayerRawPlayers() const;

	// 後始末
	void ClearPlayers();

	// 勝敗判定
	void CheckGameResult();

	// プレイヤー順位（死亡順に基づく）
	std::vector<int> GetPlayerRanks() const;

	// 状態
	bool IsGameOver() const { return isGameOver_; }
	bool GetIsGameOver() const { return isGameOver_; }
	int GetWinnerID() const { return winnerID_; }

	// ゲーム開始時刻（ms）
	int GetGameStartTimeMs() const { return gameStartTimeMs_; }

	// 衝突
	void CheckPlayerCollisions();
	void CheckBulletCollisions();

	// リセット/解放
	void Reset();
	void Release();

private:
	PlayerManager();
	~PlayerManager();

	static PlayerManager* instance_;
	std::vector<std::shared_ptr<Player>> players_;

	bool isGameOver_ = false;
	int winnerID_ = -1;

	Player* player_ = nullptr;

	int gameStartTimeMs_ = 0;
};