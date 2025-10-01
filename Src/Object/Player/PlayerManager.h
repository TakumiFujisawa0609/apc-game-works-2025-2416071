#pragma once
#include <vector>
#include <memory>
#include "Player.h"
#include "../Stage/Stage.h"

// プレイヤーの種類
enum class CharacterType
{
    Player_1,
    Player_2,
    Player_3,
    Player_4
};

class PlayerManager
{
public:
    PlayerManager() = default;
    ~PlayerManager() = default;

    // プレイヤー生成
    void CreatePlayer(CharacterType type, int id);

    // 全プレイヤー初期化
    void InitAllPlayers(void);

    // 全プレイヤー更新
    void UpdateAllPlayers(Stage& stage);

    // 全プレイヤー描画
    void DrawAllPlayers(void);

    // 全プレイヤー攻撃（デバッグ用）
    void AttackAllPlayers(void);

    // 全削除a
    void ClearPlayers(void);

	// ステージとの当たり判定
	void CheckCollWithStage(Stage& stage);

	// ステージローカル座標に変換
	static VECTOR TransformToStageLocal(const VECTOR& worldPos, const Stage& stage);

	// ワールド座標に変換
    static VECTOR TransformToWorldPos(const VECTOR& local, const Stage& stage);

private:
    std::vector<std::shared_ptr<Player>> players_;
};
