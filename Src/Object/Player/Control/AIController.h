#pragma once
#include "InputController.h"
#include <DxLib.h>

// AIController: 人間らしい振る舞いをする簡易 AI
// - ワンダー（ランダム移動）、弾回避、ステージ端/傾斜回避、攻撃判定、ジャンプ判定を行う
// - コメントは日本語で統一
class Player;

class AIController : public InputController
{
public:
	AIController();
	explicit AIController(int ownerId);
	~AIController() override = default;

	// コントローラ API
	VECTOR GetMoveInputVector() const override;
	bool IsJumpTrigger() const override;
	bool IsAttackTrigger() const override;

private:
	int ownerId_;

	// 基本パラメータ／状態
	mutable int lastChangeTimeMs_;      // 出力更新タイミング
	mutable VECTOR currentMoveVec_;    // 現在の出力ベクトル（滑らか化用）
	mutable int nextChangeIntervalMs_; // 次の更新インターバル

	// 攻撃関連
	mutable int lastAttackTimeMs_;
	mutable int attackIntervalMs_;     // 攻撃クールダウン（ms）
	mutable float attackRange_;        // 攻撃有効距離（px）
	mutable float aggression_;         // 攻撃性（0..1）

	// Wander（ランダム移動）
	mutable VECTOR wanderTarget_;      // 現在の目標点
	mutable int wanderTimerMs_;        // 目標が選ばれた時間
	mutable int wanderIntervalMs_;     // 次の目標までの時間（ms）

	// ジッター／反応遅延（人間らしさ）
	mutable int reactionDelayMs_;
	mutable float moveJitterStrength_;

	// チューニング定数（必要ならここを変更）
	static constexpr float WANDER_MIN_DISTANCE = 120.0f;   // 目的地は最低これだけ離れるように
	static constexpr float EDGE_SELECTION_PROB = 0.28f;   // 端寄り目標を選ぶ確率
	static constexpr float EDGE_MARGIN_FACTOR = 0.70f;    // 端寄りのときの内側境界比率
	static constexpr float SAFE_MARGIN_BASE = 80.0f;      // 基本的な安全マージン（px）
	static constexpr float LOOKAHEAD_DISTANCE = 72.0f;   // 先読み距離（px）
	static constexpr float SLOPE_RISK_THRESHOLD = 0.12f; // 傾斜回避感度
	static constexpr float WEIGHT_AVOID_FACTOR = 0.05f;  // 重さの影響度
	static constexpr float AVOID_STRENGTH = 0.9f;        // 避ける強さ（0..1）

	// ヘルパー関数群
	VECTOR CalcDirTuNearEnemy() const;
	bool FindNearbyBulletAndDodge(VECTOR& outDodgeDir) const;
	//void KeepInsideStage(VECTOR& inOutDesired) const;
	void PickNewWanderTarget(bool ensureFar = true) const;

	// 初期化補助
	void InitDefaults();
};