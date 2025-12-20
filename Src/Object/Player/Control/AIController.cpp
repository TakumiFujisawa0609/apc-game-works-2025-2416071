#include "AIController.h"
#include "../Common/PlayerManager.h"
#include "../Common/Player.h"
//#include "../../Bullet/BulletManager.h"
#include "../../Stage/Stage.h"
#include <cmath>
#include <cstdlib>
#include <cfloat>

// デフォルトパラメータ初期化を共通化
void AIController::InitDefaults()
{
	// 出力滑らか化
	lastChangeTimeMs_ = GetNowCount();
	currentMoveVec_ = VGet(0.0f, 0.0f, 0.0f);
	nextChangeIntervalMs_ = 300 + (std::rand() % 1200);

	// 攻撃関連
	lastAttackTimeMs_ = 0;
	attackIntervalMs_ = 500;
	attackRange_ = 600.0f; // 攻撃開始距離
	aggression_ = 0.65f;

	// ワンダー
	wanderTarget_ = VGet(0.0f, 0.0f, 0.0f);
	wanderTimerMs_ = GetNowCount();
	wanderIntervalMs_ = 800 + (std::rand() % 1200);

	// 人間らしさ
	reactionDelayMs_ = 180 + (std::rand() % 200);
	moveJitterStrength_ = 0.8f;
}

// コンストラクタ（引数なし）
AIController::AIController()
	: ownerId_(-1)
{
	InitDefaults();
}

// コンストラクタ（ownerId 指定）
AIController::AIController(int ownerId)
	: ownerId_(ownerId)
{
	InitDefaults();
}

// 最寄りの敵への方向を計算して返す（XZ 平面、正規化してスケール調整）
// ターゲットが見つからなければ currentMoveVec_ を返す
VECTOR AIController::CalcDirTuNearEnemy() const
{
	auto players = PlayerManager::GetInstance().GetPlayerRawPlayers();

	// 自分を取得
	const Player* me = nullptr;
	for (auto p : players) { if (p->GetID() == ownerId_) { me = p; break; } }
	if (!me) return currentMoveVec_;

	VECTOR myPos = me->GetPos();

	// 最も近い生存プレイヤーを探索
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

	if (!best) return currentMoveVec_;

	// 方向算出（XZ 平面のみ）
	VECTOR dir = VSub(best->GetPos(), myPos);
	dir.y = 0.0f;
	float len = VSize(dir);
	if (len <= 1e-5f) return currentMoveVec_;

	// コントローラ規約に合わせる（スケール）
	return VScale(VScale(dir, 1.0f / len), 10.5f);
}

// 近傍の弾丸を検出して回避方向を outDodgeDir に設定する。
// 見つかれば true を返す。弾の速度情報があればそれを使うとより自然。
bool AIController::FindNearbyBulletAndDodge(VECTOR& outDodgeDir) const
{

	(void)outDodgeDir; // 未使用警告回避
	return false;


	//const auto& bullets = BulletManager::GetInstance().GetBullets();
	//auto players = PlayerManager::GetInstance().GetPlayerRawPlayers();
	//const Player* me = nullptr;
	//for (auto p : players) { if (p->GetID() == ownerId_) { me = p; break; } }
	//if (!me) return false;

	//VECTOR myPos = me->GetPos();
	//const float dangerRadius = 250.0f;

	//for (const auto& b : bullets)
	//{
	//	if (!b->IsAlive()) continue;
	//	if (b->GetOwnerId() == ownerId_) continue;

	//	VECTOR bPos = b->GetPos();
	//	VECTOR diff = VSub(bPos, myPos);
	//	float dist = VSize(diff);
	//	if (dist > dangerRadius) continue;

	//	// 弾の進行方向が取れるならそれを使う（存在しない場合は fallback）
	//	bool usedVel = false;
	//	VECTOR vel = VGet(0.0f, 0.0f, 0.0f);
	//	// もし Bullet に速度 API があれば以下を有効化してください（例）:
	//	// vel = b->GetVelocity();
	//	// if (fabsf(vel.x) > 1e-6f || fabsf(vel.z) > 1e-6f) usedVel = true;

	//	VECTOR dodge;
	//	if (usedVel)
	//	{
	//		vel.y = 0.0f;
	//		dodge = VGet(-vel.z, 0.0f, vel.x);
	//	}
	//	else
	//	{
	//		dodge = VGet(-diff.z, 0.0f, diff.x);
	//	}

	//	float len = VSize(dodge);
	//	if (len <= 1e-5f) continue;
	//	outDodgeDir = VScale(dodge, 12.0f / len); // 回避強さ（調整可）
	//	return true;
	//}
	//return false;
}


// PickNewWanderTarget: 目的地を選ぶ（端寄りを選ぶ確率や最小距離を保証）
void AIController::PickNewWanderTarget(bool ensureFar /*= true*/) const
{
	Stage& stage = Stage::GetInstance();
	VECTOR center = stage.GetPos();
	float stageRadius = stage.GetCollider().radius;

	auto players = PlayerManager::GetInstance().GetPlayerRawPlayers();
	const Player* me = nullptr;
	for (auto p : players) { if (p->GetID() == ownerId_) { me = p; break; } }

	float playerRadius = (me) ? me->GetCollisionRadius() : 50.0f;
	const float safeMargin = (playerRadius * 1.5f > SAFE_MARGIN_BASE) ? (playerRadius * 1.5f) : SAFE_MARGIN_BASE;
	float maxR = stageRadius - safeMargin;
	if (maxR < 30.0f) maxR = 30.0f;

	float minDist = (ensureFar) ? WANDER_MIN_DISTANCE : 20.0f;
	VECTOR candidate = VGet(0.0f, 0.0f, 0.0f);
	bool picked = false;

	bool pickEdge = ((static_cast<float>(std::rand()) / RAND_MAX) < EDGE_SELECTION_PROB);

	for (int attempt = 0; attempt < 12; ++attempt)
	{
		float ang = (static_cast<float>(std::rand()) / RAND_MAX) * 2.0f * 3.14159265358979323846f;
		float r;
		if (pickEdge)
		{
			float inner = maxR * EDGE_MARGIN_FACTOR;
			float frac = static_cast<float>(std::rand()) / RAND_MAX;
			r = inner + frac * (maxR - inner);
		}
		else
		{
			r = (static_cast<float>(std::rand()) / RAND_MAX) * maxR;
		}
		candidate.x = center.x + cosf(ang) * r;
		candidate.y = center.y;
		candidate.z = center.z + sinf(ang) * r;

		if (!me)
		{
			picked = true;
			break;
		}
		VECTOR diff = VSub(candidate, me->GetPos());
		float d = VSize(diff);
		if (!ensureFar || d >= minDist)
		{
			picked = true;
			break;
		}
	}

	if (!picked)
	{
		float ang = (static_cast<float>(std::rand()) / RAND_MAX) * 2.0f * 3.14159265358979323846f;
		float r = maxR * 0.5f;
		candidate.x = center.x + cosf(ang) * r;
		candidate.y = center.y;
		candidate.z = center.z + sinf(ang) * r;
	}

	wanderTarget_ = candidate;
	wanderTimerMs_ = GetNowCount();
	wanderIntervalMs_ = 1200 + (std::rand() % 1200);
}

VECTOR AIController::GetMoveInputVector() const
{
	int now = GetNowCount();

	// wanderTarget の初期化・周期更新
	const float initEps = 0.001f;
	if (fabsf(wanderTarget_.x) < initEps && fabsf(wanderTarget_.z) < initEps) PickNewWanderTarget();
	if (now - wanderTimerMs_ >= wanderIntervalMs_) PickNewWanderTarget();

	// 自分のプレイヤーを取得
	auto players = PlayerManager::GetInstance().GetPlayerRawPlayers();
	const Player* me = nullptr;
	for (auto p : players) { if (p->GetID() == ownerId_) { me = p; break; } }
	if (!me)
	{
		VECTOR rnd = VGet((std::rand() % 200 - 100) / 10.0f, 0.0f, (std::rand() % 200 - 100) / 10.0f);
		return rnd;
	}

	// 目的方向を計算
	VECTOR myPos = me->GetPos();
	VECTOR dir = VSub(wanderTarget_, myPos); dir.y = 0.0f;
	float len = VSize(dir);

	VECTOR desired;
	if (len <= 20.0f)
	{
		// 目標近傍では小さく滞留しつつ別目標を選ぶ
		desired = VGet(((std::rand() % 200) - 100) / 20.0f, 0.0f, ((std::rand() % 200) - 100) / 20.0f);
		if (len <= 8.0f) { PickNewWanderTarget(); currentMoveVec_.x *= 0.3f; currentMoveVec_.z *= 0.3f; }
	}
	else
	{
		desired = VScale(VScale(dir, 1.0f / len), 10.0f);
	}

	// 弾丸回避を優先的に混ぜる
	VECTOR dodge;
	if (FindNearbyBulletAndDodge(dodge))
	{
		desired = VAdd(VScale(dodge, 1.2f), VScale(desired, 0.6f));
	}

	// ステージ内と傾斜回避の補正
	//KeepInsideStage(desired);

	// 出力の滑らか化
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

// ジャンプ判定
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
		if (horDist < 150.0f && diff.y > 40.0f) return true;
	}

	return (std::rand() % 1000) < 6;
}

// 攻撃判定
bool AIController::IsAttackTrigger() const
{
	//int now = GetNowCount();
	//if (now - lastAttackTimeMs_ < attackIntervalMs_) return false;

	//auto players = PlayerManager::GetInstance().GetPlayerRawPlayers();
	//const Player* me = nullptr;
	//for (auto p : players) { if (p->GetID() == ownerId_) { me = p; break; } }
	//if (!me) return false;

	//VECTOR myPos = me->GetPos();

	//const Player* best = nullptr;
	//float bestDistSq = FLT_MAX;
	//for (auto p : players)
	//{
	//	if (p->GetID() == ownerId_) continue;
	//	if (!p->IsAlive()) continue;
	//	VECTOR diff = VSub(p->GetPos(), myPos);
	//	float d2 = diff.x * diff.x + diff.y * diff.y + diff.z * diff.z;
	//	if (d2 < bestDistSq)
	//	{
	//		bestDistSq = d2;
	//		best = p;
	//	}
	//}
	//if (!best) return false;

	//float dist = sqrtf(bestDistSq);
	//if (dist <= attackRange_)
	//{
	//	float p = (attackRange_ - dist) / attackRange_;
	//	float chance = p * aggression_;
	//	if ((std::rand() / (float)RAND_MAX) < chance)
	//	{
	//		lastAttackTimeMs_ = now;
	//		return true;
	//	}
	//}
	return false;
}