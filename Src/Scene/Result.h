#pragma once
#include <vector>
#include <cstdint>
#include <algorithm>
#include "DxLib.h"
#include "SceneBase.h"

// 結果シーン
class Result : public SceneBase
{
public:

    static inline int ClampInt(int v, int hi) { return (v < hi) ? v : hi; }
    static inline float Clamp01(float v) { return (v < 1.0f) ? v : 1.0f; }

    Result(void);
    ~Result(void)override;

    void Init(void)override;
    void Update(void)override;
    void Draw(void)override;
    void Release(void)override;

private:
    // 表示モデルの構築（ロード・配置）
    void BuildModelsForResult();

private:
    // 勝者ID（-1 の場合はゲームオーバー）
    int playerWinID_ = -1;

    // スコア（必要なら使用）
    int playerScores_[4] = { 0, 0, 0, 0 };

    // 1人プレイかどうか
    bool singlePlayer_ = true;

    // モデルハンドル群
    std::vector<int>   modelHandles_;
    // モデル配置位置
    std::vector<VECTOR> modelPositions_;
    // モデルの一括スケール
    float modelScale_ = 1.0f;

    // 背景画像
    int bgImg_ = -1;

    // 演出用（フェード/カメラ揺れ/ガイド点滅）
    int   fadeTimerMs_ = 0;
    float cameraBobPhase_ = 0.0f;
    float promptBlinkPhase_ = 0.0f;

	// ボタン画像
	int buttonImg_ = -1;
};