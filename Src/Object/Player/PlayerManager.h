#pragma once
#include <vector>
#include <memory>
#include "Player.h"

// characterの種類を列挙
enum class CharacterType
{
	Player_1,
	Player_2,
	Player_3,
	Player_4,
	Max
};

class PlayerManager
{
private:
	// プレイヤーオブジェクトの配列
	std::vector<std::shared_ptr<Player>> players_;

public:

	// 指定されたプレイヤーを生成(種類/モデルID)
	void CreatePlayer(CharacterType type,int id);

	// 全プレイヤーの初期化
	void InitAllPlayers(void);

	// 全プレイヤーの更新
	void UpdateAllPlayers(void);

	// 全プレイヤーの描画
	void DrawAllPlayers(void);

	// 全プレイヤーの攻撃
	void AttackAllPlayers(void);
};

