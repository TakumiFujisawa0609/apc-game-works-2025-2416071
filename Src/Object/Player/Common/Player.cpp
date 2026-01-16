#include "../Control/InputController.h"
//#include "../../Bullet/BulletManager.h"
#include "../../Stage/Stage.h"
#include "../../../Utility/MatrixUtility.h"
#include "../../../Utility/AsoUtility.h"
#include "../../../Manager/InputManager.h"
#include "PlayerManager.h"
#include "Player.h"
#include <DxLib.h>

int Player::nextDeathOrder_ = 1;

Player::Player(int id, const PlayerParam& param, std::unique_ptr<InputController> controller)
    : id_(id),
    param_(param),
    controller_(std::move(controller))
{
}

Player::~Player()
{
}

void Player::Init()
{
    pos_ = { 0.0f, 0.0f, 0.0f };
    moveVec_ = { 0.0f,0.0f,0.0f };
    param_.speed = 15.0f;
    angle_ = { 0.0f, AsoUtility::Deg2RadF(180.0f), 1.0f };
    modelId_ = -1;

    inputVecNor_ = { 0.0f, 0.0f, 1.0f };

    // 弾管理
    ammoCount_ = AMMO_MAX;
    ammoRegenTimerSec_ = 0.0f;

    // 反動
    recoilForce_ = DEFAULT_RECOIL_FORCE;
    recoilVerticalBonus_ = DEFAULT_RECOIL_VERTICAL_BONUS;

    // 操作ロック
    controlLockTimerSec_ = 0.0f;

    // 持続ノックバック
    knockbackVec_ = { 0.0f, 0.0f, 0.0f };
    knockbackDecaySec_ = 0.0f;

    // チャージ/ダッシュ
    isCharging_ = false;
    chargeTimerSec_ = 0.0f;
    chargeDirWorld_ = { 0.0f,0.0f,0.0f };

    isDashing_ = false;
    dashTimerSec_ = 0.0f;
    dashCooldownSec_ = 0.0f;
    dashDirWorld_ = { 0.0f,0.0f,0.0f };
    dashImpactPower_ = 0.0f;
}

void Player::Update()
{
    Stage& stage = Stage::GetInstance();
    const float dt = 1.0f / 60.0f;

    // 操作ロックタイマー
    if (controlLockTimerSec_ > 0.0f) {
        controlLockTimerSec_ -= dt;
        if (controlLockTimerSec_ < 0.0f) controlLockTimerSec_ = 0.0f;
    }

    // ダッシュ クールダウン/タイマー
    if (dashCooldownSec_ > 0.0f) {
        dashCooldownSec_ -= dt;
        if (dashCooldownSec_ < 0.0f) dashCooldownSec_ = 0.0f;
    }
    if (isDashing_) {
        dashTimerSec_ -= dt;
        if (dashTimerSec_ <= 0.0f) {
            isDashing_ = false;
        }
    }

    MATRIX rotX = MGetRotX(stage.GetAngle().x);
    MATRIX rotZ = MGetRotZ(stage.GetAngle().z);
    MATRIX stageRotMat = MMult(rotZ, rotX);
    MATRIX invStageRotMat = MTranspose(stageRotMat);

    VECTOR stageCenter = stage.GetPos();
    float stageRadius = stage.GetCollider().radius;

    VECTOR rel = VSub(pos_, stageCenter);
    VECTOR local = VTransform(rel, invStageRotMat);
    float distLocalXZ = sqrtf(local.x * local.x + local.z * local.z);
    bool insideLocalRadius = (distLocalXZ <= stageRadius);

    // 入力方向の取得（チャージ方向にも利用）
    VECTOR worldInputVec = controller_->GetMoveInputVector();
    if (worldInputVec.x != 0.0f || worldInputVec.z != 0.0f)
    {
        inputVecNor_ = VNorm(worldInputVec);

        // チャージ中は方向を更新（デッドゾーンあり）
        if (isCharging_) {
            VECTOR dir = worldInputVec; dir.y = 0.0f;
            float len = VSize(dir);
            const float DEADZONE = 0.20f; // 左スティック向け
            if (len > DEADZONE) {
                chargeDirWorld_ = VScale(dir, 1.0f / len);
            }
        }
    }

    // ため（コントローラ LT / キーボード Shift）
    bool chargeHeld = controller_->IsDashChargeHeld();

    // チャージ処理
    if (!isFalling_ && !IsControlLocked() && dashCooldownSec_ <= 0.0f && !isDashing_)
    {
        if (chargeHeld)
        {
            if (!isCharging_) {
                isCharging_ = true;
                chargeTimerSec_ = 0.0f;
                chargeDirWorld_ = inputVecNor_; // 入力がなければ最後の向き
            }
            chargeTimerSec_ += dt;
            if (chargeTimerSec_ > CHARGE_MAX_SEC) chargeTimerSec_ = CHARGE_MAX_SEC;
        }
        else
        {
            // リリースでダッシュ開始
            if (isCharging_)
            {
                float t = chargeTimerSec_;
                isCharging_ = false;
                chargeTimerSec_ = 0.0f;

                if (t >= CHARGE_MIN_SEC)
                {
                    // 方向がゼロなら最後の入力方向を使用
                    VECTOR dashDir = chargeDirWorld_;
                    float dlen = VSize(dashDir);
                    if (dlen <= 1e-5f) dashDir = inputVecNor_;
                    dlen = VSize(dashDir);
                    if (dlen <= 1e-5f) dashDir = VGet(0.0f, 0.0f, 1.0f);

                    // ステージ接線に投影→正規化（ワールド）
                    VECTOR localDir = VTransform(dashDir, invStageRotMat);
                    localDir.y = 0.0f;
                    float l = VSize(localDir);
                    if (l > 1e-5f) localDir = VScale(localDir, 1.0f / l);
                    VECTOR worldTangent = VTransform(localDir, stageRotMat);
                    float wlen = VSize(worldTangent);
                    if (wlen > 1e-5f) worldTangent = VScale(worldTangent, 1.0f / wlen);

                    // ため時間でスケール
                    float ratio = t / CHARGE_MAX_SEC; if (ratio < 0.0f) ratio = 0.0f; if (ratio > 1.0f) ratio = 1.0f;
                    float dashSpeed = DASH_BASE_SPEED + (DASH_MAX_SPEED - DASH_BASE_SPEED) * ratio;
                    float dashDur = DASH_BASE_DURATION + (DASH_MAX_DURATION - DASH_BASE_DURATION) * ratio;

                    // ヒット強度（ダッシュ専用ノックバック用の係数）
                    dashImpactPower_ = 120000.0f + (280000.0f * ratio); // 12万　40万相当

                    // 開始
                    dashDirWorld_ = worldTangent;
                    isDashing_ = true;
                    dashTimerSec_ = dashDur;
                    dashCooldownSec_ = DASH_COOLDOWN_SEC;

                    // 直ちに水平速度を上書き（初速付与）
                    moveVec_.x = dashDirWorld_.x * dashSpeed;
                    moveVec_.z = dashDirWorld_.z * dashSpeed;
                }
            }
        }
    }
    else
    {
        // 不可状態で押しっぱなしならチャージ解除
        if (!chargeHeld && isCharging_) {
            isCharging_ = false;
            chargeTimerSec_ = 0.0f;
        }
    }

    if (!isFalling_)
    {
        // チャージ中は移動入力を停止（その場でためる）
        if (!IsControlLocked() && !isDashing_ && !isCharging_) {
            Move();
        }

        // 重力
        moveVec_.y -= GRAVITY_ACCEL;

        // 摩擦（チャージ/ダッシュ/ノックバックで少し緩め）
        float frictionScale = PLAYER_FRICTION;
        if (isCharging_)   frictionScale = PLAYER_FRICTION * 0.95f;
        if (isDashing_)    frictionScale = PLAYER_FRICTION * 0.90f;
        if (knockbackDecaySec_ > 0.0f) frictionScale = PLAYER_FRICTION * 0.70f;
        moveVec_ = VScale(moveVec_, frictionScale);

        // ノックバック合成
        if (knockbackDecaySec_ > 0.0f) {
            moveVec_ = VAdd(moveVec_, VScale(knockbackVec_, 0.70f));
            knockbackVec_ = VScale(knockbackVec_, 0.88f);
            knockbackDecaySec_ -= dt;
            if (knockbackDecaySec_ <= 0.0f) {
                knockbackDecaySec_ = 0.0f;
                knockbackVec_ = { 0.0f, 0.0f, 0.0f };
            }
        }

        // ダッシュ中の最低水平速度の維持（簡易）
        if (isDashing_) {
            float hv = sqrtf(moveVec_.x * moveVec_.x + moveVec_.z * moveVec_.z);
            float minDash = DASH_BASE_SPEED * 0.8f;
            if (hv < minDash) {
                moveVec_.x = dashDirWorld_.x * minDash;
                moveVec_.z = dashDirWorld_.z * minDash;
            }
        }

        // 最大速度制限（ダッシュ　ノックバック中は少し緩め）
        float len = VSize(moveVec_);
        float maxSpeedNow = param_.maxSpeed;
        if (isDashing_)              maxSpeedNow = DASH_MAX_SPEED * 1.3f;
        else if (knockbackDecaySec_ > 0.0f) maxSpeedNow = param_.maxSpeed * 2.0f;

        if (len > maxSpeedNow) {
            moveVec_ = VScale(moveVec_, maxSpeedNow / len);
        }
    }
    else
    {
        moveVec_.y -= GRAVITY_ACCEL;
    }

    // 位置更新
    pos_ = VAdd(pos_, moveVec_);

    // ステージ衝突（最深面のみ）
    bool grounded = false;
    MV1_COLL_RESULT_POLY_DIM result =
        MV1CollCheck_Sphere(stage.GetModelID(), -1, pos_, collisionRadius_);
    if (result.HitNum > 0)
    {
        float maxDepth = 0.0f;
        int bestIdx = -1;
        for (int i = 0; i < result.HitNum; ++i)
        {
            const auto& poly = result.Dim[i];
            VECTOR toCenter = VSub(pos_, poly.HitPosition);
            float penetration = VDot(toCenter, poly.Normal);
            float depth = collisionRadius_ - penetration;
            if (depth > maxDepth) { maxDepth = depth; bestIdx = i; }
        }
        if (bestIdx >= 0 && maxDepth > 0.0f)
        {
            const auto& poly = result.Dim[bestIdx];
            VECTOR push = VScale(poly.Normal, maxDepth);
            pos_ = VAdd(pos_, push);

            if (poly.Normal.y > PUSHBACK_THRESHOLD_Y)
            {
                grounded = true;
                if (moveVec_.y < 0.0f) moveVec_.y = 0.0f;
            }

            float speedOnNormal = VDot(moveVec_, poly.Normal);
            if (speedOnNormal < 0.0f)
            {
                VECTOR projN = VScale(poly.Normal, speedOnNormal);
                moveVec_ = VSub(moveVec_, projN);
            }
        }
    }

    // 落下状態
    if (!grounded && !insideLocalRadius) isFalling_ = true;
    else if (grounded)                   isFalling_ = false;

    // 下限Y死亡
    if (pos_.y < -1000.0f) Die();

    // 弾の時間回復（ゲーム開始30秒後、勝敗未決のみ）
    {
        PlayerManager& pm = PlayerManager::GetInstance();
        if (!pm.GetIsGameOver())
        {
            int nowMs = GetNowCount();
            int startMs = pm.GetGameStartTimeMs();
            if (nowMs - startMs >= 30000)
            {
                ammoRegenTimerSec_ += dt;
                if (ammoRegenTimerSec_ >= AMMO_REGEN_INTERVAL_SEC)
                {
                    ammoRegenTimerSec_ = 0.0f;
                    if (ammoCount_ < AMMO_MAX) ammoCount_++;
                }
            }
        }
    }

    // 弾更新
    //BulletManager::GetInstance().Update();

    // モデル更新
    if (modelId_ != -1)
    {
        MV1SetPosition(modelId_, pos_);
        float rotY = atan2f(-inputVecNor_.x, -inputVecNor_.z);
        MV1SetRotationXYZ(modelId_, VGet(0.0f, rotY, 0.0f));
    }
}

void Player::Move()
{
    Stage& stage = Stage::GetInstance();
    VECTOR stageAngle = stage.GetAngle();

    MATRIX rotX = MGetRotX(stageAngle.x);
    MATRIX rotZ = MGetRotZ(stageAngle.z);
    MATRIX stageRotMat = MMult(rotZ, rotX);
    MATRIX invStageRotationMat = MTranspose(stageRotMat);

    VECTOR worldInputVec = controller_->GetMoveInputVector();

    if (worldInputVec.x != 0.0f || worldInputVec.z != 0.0f)
    {
        inputVecNor_ = VNorm(worldInputVec);
    }

    VECTOR slopeDir = VGet(sinf(stageAngle.z), 0.0f, sinf(stageAngle.x));
    float uphillFactor = VDot(inputVecNor_, slopeDir);

    float moveSpeed = param_.speed;

    VECTOR inputAcc = AsoUtility::VECTOR_ZERO;
    if (worldInputVec.x != 0.0f || worldInputVec.z != 0.0f)
    {
        VECTOR moveDir = VTransform(inputVecNor_, invStageRotationMat);
        moveDir.y = 0.0f;

        // 基本加速
        inputAcc = VScale(moveDir, moveSpeed * INPUT_ACCEL_FACTOR);

        // 上り補正（控えめ）
        if (uphillFactor > 0.0f) {
            float boost = uphillFactor * 0.3f;
            inputAcc = VAdd(inputAcc, VScale(moveDir, boost * moveSpeed * INPUT_ACCEL_FACTOR));
        }
    }

    const float GRAVITY = 9.8f;
    const float SLIDE_FACTOR = 0.5f;

    VECTOR worldGravity = VGet(0.0f, -GRAVITY * SLIDE_FACTOR, 0.0f);
    VECTOR slideAcc = VTransform(worldGravity, invStageRotationMat);
    slideAcc.y = 0.0f;

    moveVec_ = VAdd(moveVec_, inputAcc);
    moveVec_ = VAdd(moveVec_, slideAcc);

    if (controller_->IsJumpTrigger())
    {
        if (moveVec_.y <= 0.1f)
        {
            moveVec_.y = param_.jumpPower;
        }
    }
}

bool WorldToScreen(const VECTOR& world, int& outX, int& outY)
{
    // ビュー・プロジェクション行列取得
    MATRIX view = GetCameraViewMatrix();
    MATRIX proj = GetCameraProjectionMatrix();

    // ワールド→ビュー変換
    VECTOR v = VTransform(world, view);
    // ビュー→クリップ空間変換
    VECTOR clip;
    clip.x = v.x * proj.m[0][0] + v.y * proj.m[1][0] + v.z * proj.m[2][0] + proj.m[3][0];
    clip.y = v.x * proj.m[0][1] + v.y * proj.m[1][1] + v.z * proj.m[2][1] + proj.m[3][1];
    clip.z = v.x * proj.m[0][2] + v.y * proj.m[1][2] + v.z * proj.m[2][2] + proj.m[3][2];
    float w = v.x * proj.m[0][3] + v.y * proj.m[1][3] + v.z * proj.m[2][3] + proj.m[3][3];

    if (w == 0.0f) return false;

    // 正規化デバイス座標
    float ndcX = clip.x / w;
    float ndcY = clip.y / w;
    // float ndcZ = clip.z / w; // Z値が必要なら

    // 画面サイズ取得
    int screenW, screenH;
    GetScreenState(&screenW, &screenH, NULL);

    // スクリーン座標へ変換
    outX = static_cast<int>((ndcX * 0.5f + 0.5f) * screenW);
    outY = static_cast<int>((-ndcY * 0.5f + 0.5f) * screenH);

    // 画面外判定（必要なら）
    if (w < 0.0f) return false;

    return true;
}

void Player::Draw()
{
    MV1SetPosition(modelId_, pos_);

    float rotY = atan2f(-inputVecNor_.x, -inputVecNor_.z);
    VECTOR rot = { 0.0f, rotY, 0.0f };
    MV1SetRotationXYZ(modelId_, rot);

    if (pos_.y >= -1000.0f) MV1DrawModel(modelId_);

    // チャージゲージ描画
    if (isCharging_ || chargeTimerSec_ > 0.0f) {
        const float minSec = CHARGE_MIN_SEC;
        const float maxSec = CHARGE_MAX_SEC;
        float ratio = chargeTimerSec_ / maxSec;
        if (ratio > 1.0f) ratio = 1.0f;
        if (ratio < 0.0f) ratio = 0.0f;

        VECTOR gaugePos = pos_;
        gaugePos.y += 2.0f;
        gaugePos.x += 1.0f;

        int screenX, screenY;
        if (WorldToScreen(gaugePos, screenX, screenY)) {
            int barWidth = 80;
            int barHeight = 12;
            int filledWidth = static_cast<int>(barWidth * ratio);

            DrawBox(screenX, screenY, screenX + barWidth, screenY + barHeight, GetColor(80, 80, 80), FALSE);

            int minLineX = screenX + static_cast<int>(barWidth * (minSec / maxSec));
            DrawLine(minLineX, screenY, minLineX, screenY + barHeight, GetColor(255, 255, 0));

            int r = static_cast<int>(255 * (1.0f - ratio));
            int g = static_cast<int>(255 * ratio);
            int b = 0;
            DrawBox(screenX, screenY, screenX + filledWidth, screenY + barHeight, GetColor(r, g, b), TRUE);

            // クールタイム表示
            if (dashCooldownSec_ > 0.0f) {
                char cooldownText[32];
                sprintf_s(cooldownText, "CT: %.1fs", dashCooldownSec_);
                int textY = screenY + barHeight + 4;
                DrawString(screenX, textY, cooldownText, GetColor(200, 200, 255));
            }
        }
    }

#ifdef _DEBUG
    char buffer[160];
    //sprintf_s(buffer, "P%d Spd=%.1f Charge=%.2fs %s Dash=%.2fs",
    //    id_ + 1, VSize(moveVec_), chargeTimerSec_,
    //    isCharging_ ? "CHG" : "   ",
    //    isDashing_ ? dashTimerSec_ : 0.0f);
    //DrawString(10, 30 + id_ * 20, buffer, GetColor(255, 255, 255));
#endif
}

void Player::ApplyHit(const VECTOR& knockBack)
{
    // ダッシュ中は解除
    isDashing_ = false;
    dashTimerSec_ = 0.0f;

    // 持続ノックバック
    knockbackVec_ = knockBack;
    knockbackDecaySec_ = 0.18f;

    if (knockBack.y > 0.0f && knockBack.y > moveVec_.y) moveVec_.y = knockBack.y;

    // 操作ロック
    float kbHV = sqrtf(knockBack.x * knockBack.x + knockBack.z * knockBack.z);
    float lock = 0.12f + (kbHV / 3000.0f);
    if (lock > 0.8f) lock = 0.8f;
    if (lock > controlLockTimerSec_) controlLockTimerSec_ = lock;

    // 速度キャップ
    float hv = sqrtf(moveVec_.x * moveVec_.x + moveVec_.z * moveVec_.z);
    const float MAX_HORIZONTAL_SPEED = 900.0f;
    if (hv > MAX_HORIZONTAL_SPEED) {
        float s = MAX_HORIZONTAL_SPEED / hv;
        moveVec_.x *= s;
        moveVec_.z *= s;
    }
    const float MAX_VERTICAL_SPEED = 1200.0f;
    if (moveVec_.y > MAX_VERTICAL_SPEED)  moveVec_.y = MAX_VERTICAL_SPEED;
    if (moveVec_.y < -MAX_VERTICAL_SPEED) moveVec_.y = -MAX_VERTICAL_SPEED;
}

void Player::Die()
{
    if (!isAlive_) return;
    isAlive_ = false;
    deathOrder_ = nextDeathOrder_++;
}

void Player::Release()
{
	if (modelId_ != -1) {
		MV1DeleteModel(modelId_);
		modelId_ = -1;
	}
}

