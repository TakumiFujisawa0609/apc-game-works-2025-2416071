#pragma once
#include <vector>
#include <memory>
#include "Player.h"
#include "../Stage/Stage.h"

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

	// インスタンス生成
	static void CreateInstance();

	// インスタンス取得
	static PlayerManager& GetInstance();

	// 初期化
	void Init();

	// 全プレイヤー初期化
	void InitAllPlayers();


	// プレイヤー生成
	void CreatePlayer(PlayerType type,int id, float weight,int inputId);

	// 全プレイヤー更新
	void UpdatePlayers(Stage& stage);

	// 全プレイヤー描画
	void DrawPlayers();

	// 生データのプレイヤー配列を取得
	std::vector<Player*> GetPlayerRawPlayers() const;

	// 全削除
	void ClearPlayers();

	// ステージとの当たり判定
	//void CheckCollWithStage(Stage& stage);

private:

	PlayerManager();
	~PlayerManager();
	// インスタンス
	static PlayerManager* instance_;
	std::vector<std::shared_ptr<Player>> players_;

};