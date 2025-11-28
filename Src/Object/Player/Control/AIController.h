#pragma once
#include "InputController.h"
#include <DxLib.h>

class AIController : public InputController
{
public:
	AIController();
	explicit AIController(int ownerId);
	~AIController() override = default;

	VECTOR GetMoveInputVector() const override;
	bool IsJumpTrigger() const override;
	bool IsAttackTrigger() const override;

private:
	int ownerId_;

	// 時間制御・現在の出力ベクタなど
	mutable int lastChangeTimeMs_;
	mutable VECTOR currentMoveVec_;
	mutable int nextChangeIntervalMs_;
	mutable int lastAttackTimeMs_;
	mutable int attackIntervalMs_;

	// Wander (ランダム移動) 用状態
	mutable VECTOR wanderTarget_;
	mutable int wanderTimerMs_;
	mutable int wanderIntervalMs_;

	// ヘルパー
	VECTOR CalcDirTuNearEnemy() const; // 使わないが残す（将来の拡張用）
	bool FindNearbyBulletAndDodge(VECTOR& outDodgeDir) const;
	void KeepInsideStage(VECTOR& inOutDesired) const;

	// 新規: wander 用ヘルパー
	void PickNewWanderTarget() const;
};