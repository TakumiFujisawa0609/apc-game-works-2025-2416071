#include "AIController.h"
#include "../Common/PlayerManager.h"
#include "../Common/Player.h"
#include "../../Bullet/BulletManager.h"
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
	attackRange_ = 600.0f; // 攻撃開始距離（拡張済み）
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

		// 弾の進行方向が取れるならそれを使う（存在しない場合は fallback）
		bool usedVel = false;
		VECTOR vel = VGet(0.0f, 0.0f, 0.0f);
		// もし Bullet に速度 API があれば以下を有効化してください（例）:
		// vel = b->GetVelocity();
		// if (fabsf(vel.x) > 1e-6f || fabsf(vel.z) > 1e-6f) usedVel = true;

		VECTOR dodge;
		if (usedVel)
		{
			vel.y = 0.0f;
			dodge = VGet(-vel.z, 0.0f, vel.x);
		}
		else
		{
			dodge = VGet(-diff.z, 0.0f, diff.x);
		}

		float len = VSize(dodge);
		if (len <= 1e-5f) continue;
		outDodgeDir = VScale(dodge, 12.0f / len); // 回避強さ（調整可）
		return true;
	}
	return false;
}

// ステージ内にとどまる・傾斜を避けるための補正を inOutDesired に適用する
//void AIController::KeepInsideStage(VECTOR& inOutDesired) const
//{
//	Stage& stage = Stage::GetInstance();
//	VECTOR stageCenter = stage.GetPos();
//	float stageRadius = stage.GetCollider().radius;
//
//	auto players = PlayerManager::GetInstance().GetPlayerRawPlayers();
//	const Player* me = nullptr;
//	for (auto p : players) { if (p->GetID() == ownerId_) { me = p; break; } }
//	if (!me) return;
//
//	VECTOR myPos = me->GetPos();
//
//	// 小さいベクトルなら中央へバイアス
//	const float eps = 1e-5f;
//	if (fabsf(inOutDesired.x) < eps && fabsf(inOutDesired.z) < eps)
//	{
//		VECTOR toCenter = VSub(stageCenter, myPos);
//		toCenter.y = 0.0f;
//		float lenToCenter = VSize(toCenter);
//		if (lenToCenter > eps)
//		{
//			inOutDesired = VScale(VScale(toCenter, 1.0f / lenToCenter), 12.0f);
//		}
//		return;
//	}
//
//	// プレイヤー半径とセーフマージンの計算（std::min/std::max を使わない）
//	float playerRadius = me->GetCollisionRadius();
//	const float safeMargin = (playerRadius * 1.5f > SAFE_MARGIN_BASE) ? (playerRadius * 1.5f) : SAFE_MARGIN_BASE;
//
//	// 予測位置を計算して境界近傍か判定
//	VECTOR desiredNorm = VNorm(inOutDesired);
//	VECTOR predictPos = VAdd(myPos, VScale(desiredNorm, LOOKAHEAD_DISTANCE));
//	VECTOR centerToPredict = VSub(predictPos, stageCenter);
//	centerToPredict.y = 0.0f;
//	float dist = sqrtf(centerToPredict.x * centerToPredict.x + centerToPredict.z * centerToPredict.z);
//
//	// 傾斜に基づく危険判定（Stage::GetAngle が存在する前提）
//	VECTOR stageAngle = Stage::GetInstance().GetAngle();
//	VECTOR slopeDir = VGet(sinf(stageAngle.z), 0.0f, sinf(stageAngle.x));
//	float slopeMag = sqrtf(slopeDir.x * slopeDir.x + slopeDir.z * slopeDir.z);
//	float downDot = 0.0f;
//	if (slopeMag > 1e-5f)
//	{
//		VECTOR dn = VNorm(slopeDir);
//		downDot = VDot(desiredNorm, dn);
//	}
//	float weight = me->GetWeight(); // Player::GetWeight() がある想定
//	float weightFactor = weight * WEIGHT_AVOID_FACTOR;
//	float slopeRisk = downDot * slopeMag * (1.0f + weightFactor);
//
//	// 予測位置が外側 or 傾斜リスクが高ければ中心方向へバイアス
//	if (dist > (stageRadius - safeMargin) || slopeRisk > SLOPE_RISK_THRESHOLD)
//	{
//		VECTOR toCenter = VSub(stageCenter, myPos); toCenter.y = 0.0f;
//		float len = VSize(toCenter);
//		if (len <= 1e-5f) return;
//		VECTOR toCenterNorm = VScale(toCenter, 1.0f / len);
//
//		// 回避混合係数をランダムで少し揺らす（自然さ）
//		float avoidMix = AVOID_STRENGTH * (0.85f + (std::rand() % 31) / 100.0f);
//		if (avoidMix > 1.0f) avoidMix = 1.0f;
//		inOutDesired = VAdd(VScale(inOutDesired, 1.0f - avoidMix), VScale(toCenterNorm, 12.0f * avoidMix));
//
//		// 傾斜が特に強い場合は速度（大きさ）も落とす
//		if (slopeRisk > SLOPE_RISK_THRESHOLD * 1.2f)
//		{
//			float cap = (slopeRisk * 1.2f < 0.75f) ? (slopeRisk * 1.2f) : 0.75f;
//			float slowFactor = 1.0f - cap; // 最大で 0.25 倍にする等の調整
//			inOutDesired = VScale(inOutDesired, slowFactor);
//		}
//		return;
//	}
//
//	// 近傍境界の微補正（外向き成分を抑える）
//	{
//		VECTOR future = VAdd(myPos, VScale(VNorm(inOutDesired), LOOKAHEAD_DISTANCE));
//		VECTOR cf = VSub(future, stageCenter); cf.y = 0.0f;
//		float fd = sqrtf(cf.x * cf.x + cf.z * cf.z);
//		if (fd > (stageRadius - safeMargin * 0.5f))
//		{
//			VECTOR toCenter = VSub(stageCenter, myPos);
//			toCenter.y = 0.0f;
//			float tl = VSize(toCenter);
//			if (tl > 1e-5f)
//			{
//				VECTOR toCenterNorm = VScale(toCenter, 1.0f / tl);
//				float outwardComp = VDot(VNorm(inOutDesired), VNorm(VSub(myPos, stageCenter)));
//				if (outwardComp > 0.1f)
//				{
//					inOutDesired = VAdd(inOutDesired, VScale(toCenterNorm, 6.0f));
//				}
//			}
//		}
//	}
//}

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

// GetMoveInputVector の実装（ワンダー + 回避 + ステージ補正 + 滑らか化）
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

// ジャンプ判定（高い敵が近ければジャンプ、でなければ低確率）
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

// 攻撃判定（拡張された attackRange_ を使用、距離に応じて確率を下げる）
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

	float dist = sqrtf(bestDistSq);
	if (dist <= attackRange_)
	{
		float p = (attackRange_ - dist) / attackRange_;
		float chance = p * aggression_;
		if ((std::rand() / (float)RAND_MAX) < chance)
		{
			lastAttackTimeMs_ = now;
			return true;
		}
	}
	return false;
}