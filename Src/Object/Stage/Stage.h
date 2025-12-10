#pragma once
#include <DxLib.h>
#include <vector>
#include <algorithm>
#include "Tile.h"
class Player; // 前方宣言

// シリンダー型コライダー
struct CylinderCollider
{
    VECTOR center;    // 中心位置
    float radius;     // 半径
    float yMin;       // Yの最小値
    float yMax;       // Yの最大値
};

class Stage
{
public:
    // 定数
    static constexpr VECTOR DEFAULT_POS = { 0.0f, 0.0f, 0.0f }; // ステージの初期位置
    static constexpr VECTOR DEFAULT_SCALE = { 3.0f, 3.0f, 3.0f };
    static constexpr float COLLIDER_RADIUS = 1000.0f;
    static constexpr float COLLIDER_YMAX_OFFSET = 5000.0f;
    static constexpr float MOMENT_OF_INERTIA = 10000.0f;
    static constexpr float DAMPING_FACTOR = 0.05f;

    // タイル
    static constexpr int TILE_COUNT = 7;
    static constexpr float TILE_SIZE = 200.0f;

    // インスタンス
    static void CreateInstance();
    static Stage& GetInstance();

    // 基本処理
    void Init();
    void Update(const std::vector<Player*>& players);
    void Draw();
    void Release();

    // モデルID取得
    int GetModelID() const { return tileModelId_; }

    // ステージの位置・角度・スケール取得
    const VECTOR& GetPos() const { return pos_; }
    const VECTOR& GetAngle() const { return angle_; }
    const VECTOR& GetScale() const { return scale_; }

    // コライダー取得
    const CylinderCollider& GetCollider() const { return collider_; }

    // 傾き/物理系
    void UpdateTilt(const std::vector<Player*>& players);
    bool IsPlayerOnStage(const VECTOR& playerPos) const;
    VECTOR GetStageNormal() const;

    // タイル系
    bool WorldToTileIndex(const VECTOR& worldPos, int& outTileX, int& outTileZ) const;
    VECTOR TileIdxToWorld(int x, int z) const;

    // プレイヤーが踏んだ位置を確認
    void CheckPlayerStepOnTiles(const std::vector<Player*>& players);

    // タイル配列への安全なアクセス(Getter)
    Tile& GetTile(int x, int z) { return tiles_[x][z]; }
    const Tile& GetTile(int x, int z) const { return tiles_[x][z]; }
 

private:

    // タイル配列
    Tile tiles_[TILE_COUNT][TILE_COUNT];

    Stage() = default;
    ~Stage() = default;

    static Stage* instance_;

    // モデルID
    int modelId_ = -1;
    int skyModelId_ = -1;
    int tileModelId_ = -1;

    // 位置・角度・スケール
    VECTOR pos_ = DEFAULT_POS;
    VECTOR angle_{};
    VECTOR scale_ = DEFAULT_SCALE;

    // スカイドーム
    VECTOR skyPos_{};
    VECTOR skyScale_{};
    VECTOR skyAngle_{};

    // コライダー
    CylinderCollider collider_{};

    // 物理制御
    VECTOR angularVelocity_{};
    float momentOfInertia_ = MOMENT_OF_INERTIA;
    float dampingFactor_ = DAMPING_FACTOR;
    float restitutionFactor_ = 0.0f;

    float maxStageRange_ = 600.0f;


};