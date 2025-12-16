#pragma once
#include "InputController.h"
#include <DxLib.h>

// AIController: プレイヤーらしさのある簡易AI
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

    // 追加: AI はデフォルトでためない
    bool IsDashChargeHeld() const override { return false; }

private:
    int ownerId_;

    // 既存パラメータ…
    mutable int   lastChangeTimeMs_;
    mutable VECTOR currentMoveVec_;
    mutable int   nextChangeIntervalMs_;

    mutable int   lastAttackTimeMs_;
    mutable int   attackIntervalMs_;
    mutable float attackRange_;
    mutable float aggression_;

    mutable VECTOR wanderTarget_;
    mutable int   wanderTimerMs_;
    mutable int   wanderIntervalMs_;

    mutable int   reactionDelayMs_;
    mutable float moveJitterStrength_;

    // 調整用定数
    static constexpr float WANDER_MIN_DISTANCE = 120.0f;
    static constexpr float EDGE_SELECTION_PROB = 0.28f;
    static constexpr float EDGE_MARGIN_FACTOR = 0.70f;
    static constexpr float SAFE_MARGIN_BASE = 80.0f;
    static constexpr float LOOKAHEAD_DISTANCE = 72.0f;
    static constexpr float SLOPE_RISK_THRESHOLD = 0.12f;
    static constexpr float WEIGHT_AVOID_FACTOR = 0.05f;
    static constexpr float AVOID_STRENGTH = 0.9f;

    VECTOR CalcDirTuNearEnemy() const;
    bool FindNearbyBulletAndDodge(VECTOR& outDodgeDir) const;
    void PickNewWanderTarget(bool ensureFar = true) const;

    void InitDefaults();
};