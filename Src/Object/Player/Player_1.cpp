#include "Player_1.h"
#include <DxLib.h>

void Player_1::Init(void)
{

}

void Player_1::Update(void)
{
	// コントローラの状態を取得

}
void Player_1::Attack(void)
{
	//攻撃処理
}

void Player_1::Draw(void)
{
	// デバッグ
	DrawFormatString(400, 0, GetColor(255, 255, 255), "Player_1");

}

// プレイヤーの情報
VECTOR playerPos = VGet(0.0f, 200.0f, 0.0f); // 中心（腰あたり）
float radius = 20.0f;   // 当たり判定の半径
float height = 80.0f;   // プレイヤーの高さ
float speed = 5.0f;

void UpdatePlayer(int stageModelId)
{
    VECTOR move = VGet(0.0f, 0.0f, 0.0f);

    // 入力による移動方向
    if (CheckHitKey(KEY_INPUT_W)) move.z -= 1.0f;
    if (CheckHitKey(KEY_INPUT_S)) move.z += 1.0f;
    if (CheckHitKey(KEY_INPUT_A)) move.x -= 1.0f;
    if (CheckHitKey(KEY_INPUT_D)) move.x += 1.0f;

    // 正規化して速度をかける
    if (move.x != 0.0f || move.z != 0.0f)
    {
        move = VNorm(move);
        move = VScale(move, speed);
    }

    // 移動後の座標を仮計算
    VECTOR nextPos = VAdd(playerPos, move);

    // ---- カプセルで衝突判定 ----
    VECTOR capsuleStart = VAdd(nextPos, VGet(0.0f, -height / 2, 0.0f)); // 足元
    VECTOR capsuleEnd = VAdd(nextPos, VGet(0.0f, height / 2, 0.0f)); // 頭

    MV1_COLL_RESULT_POLY result = MV1CollCheck_Capsule(
        stageModelId, -1,
        capsuleStart, capsuleEnd, radius
    );

    if (result.HitFlag == TRUE)
    {
        // めり込んだら押し戻す
        nextPos = VAdd(nextPos, VScale(result.Normal, result.Dist));
    }

    // ---- 足元をレイキャストして地面に立たせる ----
    VECTOR start = nextPos;
    VECTOR end = VAdd(nextPos, VGet(0.0f, -1000.0f, 0.0f));

    MV1_COLL_RESULT_POLY ground = MV1CollCheck_Line(stageModelId, -1, start, end);
    if (ground.HitFlag == TRUE)
    {
        nextPos.y = ground.HitPosition.y + height / 2.0f; // 足元を地面に
    }

    // 更新
    playerPos = nextPos;

    // 判定メモリ解放
    MV1CollResultPolyDimTerminate(result);
    MV1CollResultPolyDimTerminate(ground);
}

