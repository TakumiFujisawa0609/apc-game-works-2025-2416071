#include "AIController.h"
#include "../Common/PlayerManager.h"
#include "../../Bullet/BulletManager.h"
#include "../../Stage/Stage.h"
#include <cmath>
#include <cstdlib>
#include <cfloat>

// デフォルト & ownerId コンストラクタ
AIController::AIController()
	: ownerId_(-1)
	, lastChangeTimeMs_(GetNowCount())
	, currentMoveVec_{ 0.0f, 0.0f, 0.0f }
	, nextChangeIntervalMs_(300 + (std::rand() % 1200))
	, lastAttackTimeMs_(0)
	, attackIntervalMs_(500)
	, wanderTarget_{ 0.0f, 0.0f, 0.0f }
	, wanderTimerMs_(GetNowCount())
	, wanderIntervalMs_(800 + (std::rand() % 1200))
{
}

AIController::AIController(int ownerId)
	: ownerId_(ownerId)
	, lastChangeTimeMs_(GetNowCount())
	, currentMoveVec_{ 0.0f, 0.0f, 0.0f }
	, nextChangeIntervalMs_(300 + (std::rand() % 1200))
	, lastAttackTimeMs_(0)
	, attackIntervalMs_(500)
	, wanderTarget_{ 0.0f, 0.0f, 0.0f }
	, wanderTimerMs_(GetNowCount())
	, wanderIntervalMs_(800 + (std::rand() % 1200))
{
}

// （既存の CalcDirTuNearEnemy / FindNearbyBulletAndDodge / KeepInsideStage は流用）
VECTOR AIController::CalcDirTuNearEnemy() const
{
	// 既存の追跡ロジック（残すが今回の wander では使用しない）
	auto players = PlayerManager::GetInstance().GetPlayerRawPlayers();
	const Player* me = nullptr;
	for (auto p : players) { if (p->GetID() == ownerId_) { me = p; break; } }
	if (!me) return currentMoveVec_;

	VECTOR myPos = me->GetPos();

	const Player* best = nullptr;
	float bestDistSq = FLT_MAX;
	for (auto p : players)
	{
		if (p->GetID() == ownerId_) continue;
		if (!p->IsAlive()) continue;
		VECTOR diff = VSub(p->GetPos(), myPos);
		float distSq = diff.x * diff.x + diff.y * diff.y + diff.z * diff.z;
		if (distSq < bestDistSq)
		{
			bestDistSq = distSq;
			best = p;
		}
	}

	if (!best) return currentMoveVec_;

	VECTOR dir = VSub(best->GetPos(), myPos);
	dir.y = 0.0f;
	float len = VSize(dir);
	if (len <= 1e-5f) return currentMoveVec_;
	return VScale(VScale(dir, 1.0f / len), 10.5f);
}

bool AIController::FindNearbyBulletAndDodge(VECTOR& outDodgeDir) const
{
	const auto& bullets = BulletManager::GetInstance().GetBullets();
	auto players = PlayerManager::GetInstance().GetPlayerRawPlayers();
	const Player* me = nullptr;
	for (auto p : players) { if (p->GetID() == ownerId_) { me = p; break; } }
	if (!me) return false;

	VECTOR myPos = me->GetPos();
	const float dangerRadius = 250.0f;

	for (const auto& b : bullets)
	{
		if (!b->IsAlive()) continue;
		if (b->GetOwnerId() == ownerId_) continue;

		VECTOR bPos = b->GetPos();
		VECTOR diff = VSub(bPos, myPos);
		float dist = VSize(diff);
		if (dist > dangerRadius) continue;

		VECTOR dodge = VGet(-diff.z, 0.0f, diff.x);
		float len = VSize(dodge);
		if (len <= 1e-5f) continue;
		outDodgeDir = VScale(dodge, 12.0f / len);
		return true;
	}
	return false;
}

void AIController::KeepInsideStage(VECTOR& inOutDesired) const
{
	Stage& stage = Stage::GetInstance();
	VECTOR stageCenter = stage.GetPos();
	float stageRadius = stage.GetCollider().radius;

	auto players = PlayerManager::GetInstance().GetPlayerRawPlayers();
	const Player* me = nullptr;
	for (auto p : players) { if (p->GetID() == ownerId_) { me = p; break; } }
	if (!me) return;

	VECTOR myPos = me->GetPos();

	const float eps = 1e-5f;
	if (fabsf(inOutDesired.x) < eps && fabsf(inOutDesired.z) < eps)
	{
		VECTOR toCenter = VSub(stageCenter, myPos);
		toCenter.y = 0.0f;
		float lenToCenter = VSize(toCenter);
		if (lenToCenter > eps)
		{
			inOutDesired = VScale(VScale(toCenter, 1.0f / lenToCenter), 12.0f);
		}
		return;
	}

	float playerRadius = me->GetCollisionRadius();
	const float baseSafeMargin = 80.0f;
	const float safeMargin = (playerRadius * 1.5f > baseSafeMargin) ? (playerRadius * 1.5f) : baseSafeMargin;

	const float lookAhead = 60.0f;
	VECTOR desiredNorm = VNorm(inOutDesired);
	VECTOR predictPos = VAdd(myPos, VScale(desiredNorm, lookAhead));

	VECTOR centerToPredict = VSub(predictPos, stageCenter);
	centerToPredict.y = 0.0f;
	float dist = sqrtf(centerToPredict.x * centerToPredict.x + centerToPredict.z * centerToPredict.z);

	if (dist > (stageRadius - safeMargin))
	{
		VECTOR toCenter = VSub(stageCenter, myPos);
		toCenter.y = 0.0f;
		float len = VSize(toCenter);
		if (len <= 1e-5f) return;
		VECTOR toCenterNorm = VScale(toCenter, 1.0f / len);

		const float inwardStrength = 1.0f;
		if (inwardStrength >= 1.0f)
		{
			inOutDesired = VScale(toCenterNorm, 12.0f);
		}
		else
		{
			inOutDesired = VAdd(VScale(inOutDesired, 1.0f - inwardStrength), VScale(toCenterNorm, 10.0f * inwardStrength));
		}
	}
}

// 新規: wander 目標を pick する（ステージ内のランダム点）
void AIController::PickNewWanderTarget() const
{
	Stage& stage = Stage::GetInstance();
	VECTOR center = stage.GetPos();
	float radius = stage.GetCollider().radius;

	// safe margin を考慮
	auto players = PlayerManager::GetInstance().GetPlayerRawPlayers();
	const Player* me = nullptr;
	for (auto p : players) { if (p->GetID() == ownerId_) { me = p; break; } }
	float playerRadius = (me) ? me->GetCollisionRadius() : 50.0f;
	const float baseSafeMargin = 80.0f;
	const float safeMargin = (playerRadius * 1.5f > baseSafeMargin) ? (playerRadius * 1.5f) : baseSafeMargin;

	// pick random polar coords inside (radius - safeMargin)
	float r = static_cast<float>(std::rand()) / RAND_MAX * (radius - safeMargin);
	float ang = (static_cast<float>(std::rand()) / RAND_MAX) * 2.0f * 3.14159265358979323846f;

	wanderTarget_.x = center.x + cosf(ang) * r;
	wanderTarget_.y = center.y; // keep same level as stage center
	wanderTarget_.z = center.z + sinf(ang) * r;

	// schedule next pick
	wanderTimerMs_ = GetNowCount();
	wanderIntervalMs_ = 800 + (std::rand() % 1200);
}

// GetMoveInputVector を wander ベースに差し替え
VECTOR AIController::GetMoveInputVector() const
{
	int now = GetNowCount();

	// wanderTarget が未初期化（0,0,0）の場合や時間経過で再設定
	const float initEps = 0.001f;
	if (fabsf(wanderTarget_.x) < initEps && fabsf(wanderTarget_.z) < initEps)
	{
		PickNewWanderTarget();
	}

	if (now - wanderTimerMs_ >= wanderIntervalMs_)
	{
		PickNewWanderTarget();
	}

	// desired は wanderTarget へ向かう方向
	auto players = PlayerManager::GetInstance().GetPlayerRawPlayers();
	const Player* me = nullptr;
	for (auto p : players) { if (p->GetID() == ownerId_) { me = p; break; } }
	if (!me)
	{
		// fallback: random small vector
		VECTOR rnd = VGet((std::rand() % 200 - 100) / 10.0f, 0.0f, (std::rand() % 200 - 100) / 10.0f);
		return rnd;
	}

	VECTOR myPos = me->GetPos();
	VECTOR dir = VSub(wanderTarget_, myPos);
	dir.y = 0.0f;
	float len = VSize(dir);

	VECTOR desired;
	if (len <= 20.0f)
	{
		// 近ければ少しスローでランダム微動作
		desired = VGet(((std::rand() % 200) - 100) / 20.0f, 0.0f, ((std::rand() % 200) - 100) / 20.0f);
	}
	else
	{
		desired = VScale(VScale(dir, 1.0f / len), 10.0f); // speed 相当
	}

	// 弾丸回避が必要な場合は優先して混ぜる
	VECTOR dodge;
	if (FindNearbyBulletAndDodge(dodge))
	{
		desired = VAdd(VScale(dodge, 1.2f), VScale(desired, 0.6f));
	}

	// ステージ内補正（強制補正を行う）
	KeepInsideStage(desired);

	// スムージング
	if (now - lastChangeTimeMs_ >= nextChangeIntervalMs_)
	{
		currentMoveVec_ = desired;
		lastChangeTimeMs_ = now;
		nextChangeIntervalMs_ = 150 + (std::rand() % 500);
	}
	else
	{
		currentMoveVec_.x = currentMoveVec_.x * 0.82f + desired.x * 0.18f;
		currentMoveVec_.z = currentMoveVec_.z * 0.82f + desired.z * 0.18f;
	}

	currentMoveVec_.y = 0.0f;
	return currentMoveVec_;
}

// IsJumpTrigger / IsAttackTrigger はこれまで通り（省略せず元のまま）
bool AIController::IsJumpTrigger() const
{
	auto players = PlayerManager::GetInstance().GetPlayerRawPlayers();
	const Player* me = nullptr;
	for (auto p : players) { if (p->GetID() == ownerId_) { me = p; break; } }
	if (!me) return false;

	VECTOR myPos = me->GetPos();

	for (auto p : players)
	{
		if (p->GetID() == ownerId_) continue;
		if (!p->IsAlive()) continue;
		VECTOR diff = VSub(p->GetPos(), myPos);
		float horDist = sqrtf(diff.x * diff.x + diff.z * diff.z);
		if (horDist < 150.0f && diff.y > 40.0f)
		{
			return true;
		}
	}

	return (std::rand() % 1000) < 6;
}

bool AIController::IsAttackTrigger() const
{
	int now = GetNowCount();

	if (now - lastAttackTimeMs_ < attackIntervalMs_) return false;

	auto players = PlayerManager::GetInstance().GetPlayerRawPlayers();
	const Player* me = nullptr;
	for (auto p : players) { if (p->GetID() == ownerId_) { me = p; break; } }
	if (!me) return false;

	VECTOR myPos = me->GetPos();

	const Player* best = nullptr;
	float bestDistSq = FLT_MAX;
	for (auto p : players)
	{
		if (p->GetID() == ownerId_) continue;
		if (!p->IsAlive()) continue;
		VECTOR diff = VSub(p->GetPos(), myPos);
		float d2 = diff.x * diff.x + diff.y * diff.y + diff.z * diff.z;
		if (d2 < bestDistSq)
		{
			bestDistSq = d2;
			best = p;
		}
	}
	if (!best) return false;

	const float attackRange = 300.0f;
	float dist = sqrtf(bestDistSq);
	if (dist <= attackRange)
	{
		float p = (attackRange - dist) / attackRange;
		int prob = static_cast<int>(p * 1000.0f);
		if ((std::rand() % 1000) < prob)
		{
			lastAttackTimeMs_ = now;
			return true;
		}
	}
	return false;
}