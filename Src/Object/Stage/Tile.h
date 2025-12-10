#pragma once
#include <DxLib.h>
#include "../../Utility/AsoUtility.h"
class Stage;

// タイルの種類
enum class TYPE
{
    NORMAL,
    BRAKABLE,
    SAFE,
};

class Tile
{
public:
    // デフォルトコンストラクタ
    Tile() = default;

    // 初期化。モデル複製パターン
    void Init(TYPE type, int hp, const VECTOR& pos, int baseModelId);

    // タイルにダメージを与える
    void Damage(int amount);

    // プレイヤーが踏んだ時
    void OnStep();

    // モデル描画
    void Draw(const VECTOR& stageAngle) const;

    bool IsHole() const { return isHole_; }
    const VECTOR& GetPos() const { return pos_; }
    TYPE GetType() const { return type_; }

    // 足元判定用: (x, z)がこのタイル領域に含まれているか
    bool HitTest(const VECTOR& pos, float radius = 10.0f) const {
        if (isHole_) return false;
        float half = 200.0f * 0.5f;
        return (pos.x > pos_.x - half - radius && pos.x < pos_.x + half + radius &&
            pos.z > pos_.z - half - radius && pos.z < pos_.z + half + radius);
    }

    // 解放
    void Release();

private:
    TYPE type_ = TYPE::NORMAL;
    int hp_ = 0;
    bool isHole_ = false;
    VECTOR pos_ = AsoUtility::VECTOR_ZERO;
    int modelId_ = -1; // ※各タイルごとのモデルハンドル

    // 踏んだ時のダメージ間隔管理
    int lastStepTime_ = 0;
    static constexpr int STEP_DAMEGE_INTERVAL = 1000; // ms
};