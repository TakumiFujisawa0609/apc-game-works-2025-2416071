#include "Player_1.h"
#include <DxLib.h>
#include "../../Utility/MatrixUtility.h"	// Lerp用
#include "../../Manager/InputManager.h"
#include "../../Utility/AsoUtility.h"

Player_1::Player_1(int id, float weight)
	: Player(id, weight)
{
}

void Player_1::Init()
{
	// 親クラスの初期化
	Player::Init();

	// モデルの読み込み
	modelId_ = MV1LoadModel("Data/Model/Player/testModel.mv1");

	pos_ = { 50.0f, 0.0f, -30.0f };
	speed_ = 5.0f;
}


// Player_1.cpp の Player_1::Move()

// ----------------------------------------------------
// 【重要】ローカル関数として VSizeSq を再定義 (DxLibの標準関数ではないため)
// ----------------------------------------------------
// Player_1.cpp の Player_1::Move()

// ----------------------------------------------------
// 【重要】ローカル関数として VSizeSq を再定義 (DxLibの標準関数ではないため)
// ----------------------------------------------------
inline static float VSizeSq(const VECTOR& v)
{
    return v.x * v.x + v.y * v.y + v.z * v.z;
}

// ----------------------------------------------------
// Player::Move() の本体
// ----------------------------------------------------
void Player_1::Move()
{
    // --- 1. 定数の定義と初期化 ---
    // ★ これらの定数を調整して、操作性を調整してください
    const float GRAVITY = 98.1f;         // 重力加速度 (ステージのGRAVITYと合わせる)
    const float SLIDE_FACTOR = 0.5f;     // 滑り落ちる力の強さ（0.1f〜1.0fで調整）
    const float PLAYER_FRICTION = 0.85f; // 摩擦による減衰（1.0fに近いほど滑り続ける）
    const float MAX_SPEED = 15.0f;       // プレイヤーの最大速度（滑落速度も含む）
    const float INPUT_ACCEL_FACTOR = 1.0f / 10.0f; // 操作による加速の強さ調整

    Stage& stage = Stage::GetInstance();
    VECTOR stageAngle = stage.GetAngle();
    InputManager& ins = InputManager::GetInstance();

    // 2. 回転行列の準備
    MATRIX rotX = MGetRotX(stageAngle.x);
    MATRIX rotZ = MGetRotZ(stageAngle.z);
    MATRIX stageRotationMatrix = MMult(rotZ, rotX);     // ステージの回転 (ローカル -> ワールド)
    MATRIX invStageRotationMatrix = MTranspose(stageRotationMatrix); // 逆回転 (ワールド -> ローカル)

    // 3. キー入力によるワールド入力ベクトル W_Input を計算
    VECTOR worldInputVec = AsoUtility::VECTOR_ZERO;
    if (ins.IsNew(KEY_INPUT_W)) worldInputVec.z += 1.0f;
    if (ins.IsNew(KEY_INPUT_S)) worldInputVec.z -= 1.0f;
    if (ins.IsNew(KEY_INPUT_A)) worldInputVec.x -= 1.0f;
    if (ins.IsNew(KEY_INPUT_D)) worldInputVec.x += 1.0f;


    // --- 4. 速度ベクトルの計算と更新 ---

    // 4.1. 傾斜による滑落加速 A_Slide の計算 (方向修正済み)

    // ワールドの重力ベクトル（下向き）
    VECTOR worldGravity = { 0.0f, -GRAVITY * SLIDE_FACTOR, 0.0f };

    // 重力ベクトルを逆行列（invStageRotationMatrix）で変換し、ステージ傾斜面における力を得る
    VECTOR slideAccel = VTransform(worldGravity, invStageRotationMatrix);

    // 垂直方向(Y)の力は無視
    slideAccel.y = 0.0f;

    // ★★★ 最終修正点: 変換されたX/Z成分を反転させ、傾斜を下る方向へ向ける ★★★
    // 変換結果が意図と逆になっているため、ここで方向を修正します。
    slideAccel.x *= -1.0f;
    slideAccel.z *= -1.0f;


    // 4.2. 操作による加速 A_Input の計算
    VECTOR inputAccel = AsoUtility::VECTOR_ZERO;
    if (worldInputVec.x != 0.0f || worldInputVec.z != 0.0f) {

        // キー入力ベクトルを傾斜を打ち消す逆行列で変換し、操作性を確保
        VECTOR actualMoveDir = VTransform(VNorm(worldInputVec), invStageRotationMatrix);
        actualMoveDir.y = 0.0f;

        // 操作による加速度を適用 (speed_を加速度の強さとして使用)
        inputAccel = VScale(actualMoveDir, speed_ * INPUT_ACCEL_FACTOR);
    }

    // 4.3. 速度の更新: 既存速度 + 滑落加速 + 操作加速

    // 既存の速度に摩擦（減衰）を適用
    moveVec_ = VScale(moveVec_, PLAYER_FRICTION);

    // 加速（滑落と操作）を加算
    moveVec_ = VAdd(moveVec_, VAdd(slideAccel, inputAccel));

    // 4.4. 最大速度のクランプ
    float currentSpeedSq = VSizeSq(moveVec_);
    if (currentSpeedSq > MAX_SPEED * MAX_SPEED)
    {
        float currentSpeed = VSize(moveVec_);
        float scaleFactor = MAX_SPEED / currentSpeed;
        moveVec_ = VScale(moveVec_, scaleFactor);
    }


    // --- 5. 位置の更新 ---

    // 最終的な速度 moveVec_ で位置を更新
    pos_ = VAdd(pos_, moveVec_);

    // 6. ジャンプ
    if (ins.IsTrgDown(KEY_INPUT_0))
    {
        pos_.y += 5.0f;
    }
}

void Player_1::DebugDraw()
{
	// プレイヤー座標を表示
	DrawFormatString(0, 500, GetColor(130, 130, 255), "Pos: (%.2f, %.2f, %.2f)", pos_.x, pos_.y, pos_.z);
}
