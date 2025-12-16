#pragma once
#include "../../Stage/Stage.h"
#include <DxLib.h>
#include <memory>

class InputController;

struct PlayerParam
{
    float weight = 10.0f;   // 重さ
    float speed = 5.0f;     // 移動速度
    float jumpPower = 5.0f; // ジャンプ力
    float friction = 0.85f; // 摩擦
    float maxSpeed = 15.0f; // 最大移動速度
};

class Player
{
public:

    // 定数
    static constexpr float MODEL_CENTER_TO_FEET = 20.0f;
    static constexpr float GRAVITY_ACCEL = 0.98f;
    static constexpr float PUSHBACK_THRESHOLD_Y = 0.5f;
    static constexpr float GRAVITY = 9.81f;
    static constexpr float SLIDE_FACTOR = 0.5f;
    static constexpr float PLAYER_FRICTION = 0.85f;
    static constexpr float MAX_SPEED = 15.0f;
    static constexpr float INPUT_ACCEL_FACTOR = 1.0f / 10.0f;
    static constexpr VECTOR INIT_VEC_NOR = { 0.0f,0.0f,1.0f };
    static constexpr float STAGE_LOWER_LIMIT_Y = -1000.0f;
    static constexpr float RESET_POS = 0.0f;

    // 弾管理
    static constexpr int   AMMO_MAX = 3;
    static constexpr float AMMO_REGEN_INTERVAL_SEC = 5.0f;

    // 反動
    static constexpr float DEFAULT_RECOIL_FORCE = 12.0f;
    static constexpr float DEFAULT_RECOIL_VERTICAL_BONUS = 0.0f;

    // チャージ/ダッシュ設定
    static constexpr float CHARGE_MAX_SEC = 1.0f;   // 最大ため時間
    static constexpr float CHARGE_MIN_SEC = 0.10f;  // ダッシュ成立に必要な最小ため時間
    static constexpr float DASH_BASE_SPEED = 50.0f;  // 最低ダッシュ速度
    static constexpr float DASH_MAX_SPEED = 110.0f; // 最大ダッシュ速度
    static constexpr float DASH_BASE_DURATION = 0.16f;  // 最低ダッシュ時間
    static constexpr float DASH_MAX_DURATION = 0.28f;  // 最大ダッシュ時間
    static constexpr float DASH_COOLDOWN_SEC = 0.6f;   // クールダウン

    bool IsControlLocked() const { return controlLockTimerSec_ > 0.0f; }
    bool IsDashing() const { return isDashing_; }
    float GetDashImpactPower() const { return dashImpactPower_; }
    const VECTOR& GetDashDir() const { return dashDirWorld_; }

    static void ResetDeathCounter() { nextDeathOrder_ = 1; }

    Player(int id, const PlayerParam& param, std::unique_ptr<InputController> controller);
    virtual ~Player();

    // 基本処理
    virtual void Init();
    virtual void Update();
    virtual void Draw();
    virtual void Release() = 0;

    // ゲッター/セッター
    int GetID() const { return id_; }
    float GetWeight() const { return param_.weight; }
    const VECTOR& GetPos() const { return pos_; }
    void SetPos(const VECTOR& newPos) { pos_ = newPos; }
    bool IsAlive() const { return isAlive_; }
    bool IsFalling() const { return isFalling_; }
    int GetDeathOrder() const { return deathOrder_; }
    float GetCollisionRadius() const { return collisionRadius_; }

    // ダッシュ終了（ヒット時に終了させる）
    void EndDashOnHit() { isDashing_ = false; dashTimerSec_ = 0.0f; }

    // ノックバック適用
    void ApplyHit(const VECTOR& knockBack);

protected:

    // 死亡
    void Die();

    // 移動
    virtual void Move();

    // 射撃
    void Shot();

    static int nextDeathOrder_;

    int deathOrder_ = 0;
    int id_;
    int modelId_ = -1;

    float collisionRadius_ = 70.0f;
    float attackCooldown_ = 0.0f;
    bool isAlive_ = true;
    bool isFalling_ = false;

    std::unique_ptr<InputController> controller_;

    VECTOR inputVecNor_;
    VECTOR angle_ = { 0.0f, 0.0f, 0.0f };
    VECTOR moveVec_;
    VECTOR pos_;

    PlayerParam param_;

    // 弾管理
    int   ammoCount_ = AMMO_MAX;
    float ammoRegenTimerSec_ = 0.0f;

    // 反動
    float recoilForce_ = DEFAULT_RECOIL_FORCE;
    float recoilVerticalBonus_ = DEFAULT_RECOIL_VERTICAL_BONUS;

    // 操作ロック（ノックバック中に入力を無効化）
    float controlLockTimerSec_ = 0.0f;

    // 持続ノックバック
    VECTOR knockbackVec_ = { 0.0f, 0.0f, 0.0f };
    float  knockbackDecaySec_ = 0.0f;

    // チャージ/ダッシュ状態
    bool   isCharging_ = false;
    float  chargeTimerSec_ = 0.0f;
    VECTOR chargeDirWorld_ = { 0.0f, 0.0f, 0.0f };

    bool   isDashing_ = false;
    float  dashTimerSec_ = 0.0f;
    float  dashCooldownSec_ = 0.0f;
    VECTOR dashDirWorld_ = { 0.0f, 0.0f, 0.0f };
    float  dashImpactPower_ = 0.0f; // ヒット時の吹き飛ばし強度（弾と独立）
};