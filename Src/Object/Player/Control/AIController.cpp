#include "AIController.h"
#include "../Common/PlayerManager.h"
#include "../Common/Player.h"
//#include "../../Bullet/BulletManager.h"
#include "../../Stage/Stage.h"
#include <cmath>
#include <cstdlib>
#include <cfloat>

// 傾斜ベクトル取得（ステージの角度から計算）
VECTOR GetStageSlopeVector()
{
    Stage& stage = Stage::GetInstance();
    VECTOR angle = stage.GetAngle();
    // x: z軸の傾き, z: x軸の傾き
    return VNorm(VGet(sinf(angle.z), 0.0f, sinf(angle.x)));
}

// 有利位置判定（坂の上方向に近いほど有利）
bool IsAdvantageousPosition(const VECTOR& pos)
{
    VECTOR slope = GetStageSlopeVector();
    // ステージ中心からposへの方向
    Stage& stage = Stage::GetInstance();
    VECTOR center = stage.GetPos();
    VECTOR dir = VNorm(VSub(pos, center));
    float dot = VDot(slope, dir);
    return dot > 0.4f; // 閾値は調整可
}

// 不利位置判定（坂の下方向に近いほど不利）
bool IsDisadvantageousPosition(const VECTOR& pos)
{
    VECTOR slope = GetStageSlopeVector();
    Stage& stage = Stage::GetInstance();
    VECTOR center = stage.GetPos();
    VECTOR dir = VNorm(VSub(pos, center));
    float dot = VDot(slope, dir);
    return dot < -0.4f;
}

// デフォルトパラメータ初期化を共通化
void AIController::InitDefaults()
{
    lastChangeTimeMs_ = GetNowCount();
    currentMoveVec_ = VGet(0.0f, 0.0f, 0.0f);
    nextChangeIntervalMs_ = 300 + (std::rand() % 1200);

    lastAttackTimeMs_ = 0;
    attackIntervalMs_ = 500;
    attackRange_ = 600.0f;
    aggression_ = 0.65f;

    wanderTarget_ = VGet(0.0f, 0.0f, 0.0f);
    wanderTimerMs_ = GetNowCount();
    wanderIntervalMs_ = 800 + (std::rand() % 1200);

    reactionDelayMs_ = 180 + (std::rand() % 200);
    moveJitterStrength_ = 0.8f;
}

AIController::AIController()
    : ownerId_(-1)
{
    InitDefaults();
}

AIController::AIController(int ownerId)
    : ownerId_(ownerId)
{
    InitDefaults();
}

VECTOR AIController::CalcDirTuNearEnemy() const
{
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
        float d2 = diff.x * diff.x + diff.y * diff.y + diff.z * diff.z;
        if (d2 < bestDistSq)
        {
            bestDistSq = d2;
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
    (void)outDodgeDir;
    return false;
}

// 坂の上方向に向かう場合を有利と判定する関数
bool IsAdvantageousDirection(const VECTOR& from, const VECTOR& to)
{
    VECTOR slope = GetStageSlopeVector();
    VECTOR dir = VNorm(VSub(to, from));
    float dot = VDot(slope, dir);
    return dot > 0.4f; // 閾値は調整可
}

// PickNewWanderTarget: 目的地を選ぶ（傾斜有利方向＋距離制限）
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
    float maxDistFromCenter = stageRadius * 0.8f; // 外周に行き過ぎないよう制限

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
        float distFromCenter = VSize(VSub(candidate, center));
        // 有利方向かつ距離条件・外周制限を満たす場合のみ選択
        if ((!ensureFar || d >= minDist) &&
            IsAdvantageousDirection(me->GetPos(), candidate) &&
            distFromCenter < maxDistFromCenter)
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

    const float initEps = 0.001f;
    if (fabsf(wanderTarget_.x) < initEps && fabsf(wanderTarget_.z) < initEps) PickNewWanderTarget();
    if (now - wanderTimerMs_ >= wanderIntervalMs_) PickNewWanderTarget();

    auto players = PlayerManager::GetInstance().GetPlayerRawPlayers();
    const Player* me = nullptr;
    for (auto p : players) { if (p->GetID() == ownerId_) { me = p; break; } }
    if (!me)
    {
        VECTOR rnd = VGet((std::rand() % 200 - 100) / 10.0f, 0.0f, (std::rand() % 200 - 100) / 10.0f);
        return rnd;
    }

    VECTOR myPos = me->GetPos();
    VECTOR dir = VSub(wanderTarget_, myPos); dir.y = 0.0f;
    float len = VSize(dir);

    VECTOR desired;
    if (len <= 20.0f)
    {
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

    // 不利位置にいる場合は坂の上方向へ補正
    if (IsDisadvantageousPosition(myPos))
    {
        VECTOR slope = GetStageSlopeVector();
        desired = VAdd(desired, VScale(slope, 8.0f));
    }

    // ステージ中心への引力を追加（外周・落下防止）
    Stage& stage = Stage::GetInstance();
    VECTOR center = stage.GetPos();
    VECTOR toCenter = VSub(center, myPos); toCenter.y = 0.0f;
    float distFromCenter = VSize(toCenter);
    float stageRadius = stage.GetCollider().radius;
    if (distFromCenter > stageRadius * 0.7f) // 外周に近い場合のみ
    {
        VECTOR pull = VScale(VNorm(toCenter), 12.0f); // 強めの補正
        desired = VAdd(desired, pull);
    }

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

bool AIController::IsAttackTrigger() const
{
    auto players = PlayerManager::GetInstance().GetPlayerRawPlayers();
    const Player* me = nullptr;
    for (auto p : players) { if (p->GetID() == ownerId_) { me = p; break; } }
    if (!me) return false;

    VECTOR myPos = me->GetPos();

    // 最も近い敵を探す
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
    // 攻撃範囲内なら攻撃
    if (dist <= attackRange_)
    {
        // 攻撃間隔（クールタイム）
        int now = GetNowCount();
        if (now - lastAttackTimeMs_ < attackIntervalMs_) return false;
        lastAttackTimeMs_ = now;
        return true;
    }
    return false;
}

