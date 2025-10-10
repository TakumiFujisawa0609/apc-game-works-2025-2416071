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

	pos_ = { 0.0f, 0.0f, -0.0f };
	speed_ = 5.0f;
}

inline static float VSizeSq(const VECTOR& v)
{
    return v.x * v.x + v.y * v.y + v.z * v.z;
}

void Player_1::Move()
{

    // ステージのインスタンス取得
    Stage& stage = Stage::GetInstance();

	// ステージの傾き（角度）を取得
    VECTOR stageAngle = stage.GetAngle();

	// 入力管理インスタンス取得
    InputManager& ins = InputManager::GetInstance();

    // 回転行列の準備
    MATRIX rotX = MGetRotX(stageAngle.x);
    MATRIX rotZ = MGetRotZ(stageAngle.z);
    MATRIX stageRotationMatrix = MMult(rotZ, rotX);     // ステージの回転 (ローカル -> ワールド)
    MATRIX invStageRotationMatrix = MTranspose(stageRotationMatrix); // 逆回転 (ワールド -> ローカル)

    // キー入力によるワールド入力ベクトル W_Input を計算
    VECTOR worldInputVec = AsoUtility::VECTOR_ZERO;
    if (ins.IsNew(KEY_INPUT_W)) worldInputVec.z += 1.0f;
    if (ins.IsNew(KEY_INPUT_S)) worldInputVec.z -= 1.0f;
    if (ins.IsNew(KEY_INPUT_A)) worldInputVec.x -= 1.0f;
    if (ins.IsNew(KEY_INPUT_D)) worldInputVec.x += 1.0f;

    // ワールドの重力ベクトル（下向き）
    VECTOR worldGravity = { 0.0f, -GRAVITY * SLIDE_FACTOR, 0.0f };

    // 重力ベクトルを逆行列（invStageRotationMatrix）で変換し、ステージ傾斜面における力を得る
    VECTOR slideAccel = VTransform(worldGravity, invStageRotationMatrix);

    // 垂直方向(Y)の力は無視
    slideAccel.y = 0.0f;

   //操作による加速 A_Input の計算
    VECTOR inputAccel = AsoUtility::VECTOR_ZERO;
    if (worldInputVec.x != 0.0f || worldInputVec.z != 0.0f) {

        // キー入力ベクトルを傾斜を打ち消す逆行列で変換し、操作性を確保
        VECTOR actualMoveDir = VTransform(VNorm(worldInputVec), invStageRotationMatrix);
        actualMoveDir.y = 0.0f;

        // 操作による加速度を適用 (speed_を加速度の強さとして使用)
        inputAccel = VScale(actualMoveDir, speed_ * INPUT_ACCEL_FACTOR);
    }

    // 既存の速度に摩擦（減衰）を適用
    moveVec_ = VScale(moveVec_, PLAYER_FRICTION);

    // 加速（滑落と操作）を加算
    moveVec_ = VAdd(moveVec_, VAdd(slideAccel, inputAccel));

 
    float currentSpeedSq = VSizeSq(moveVec_);
    if (currentSpeedSq > MAX_SPEED * MAX_SPEED)
    {
        float currentSpeed = VSize(moveVec_);
        float scaleFactor = MAX_SPEED / currentSpeed;
        moveVec_ = VScale(moveVec_, scaleFactor);
    }

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
