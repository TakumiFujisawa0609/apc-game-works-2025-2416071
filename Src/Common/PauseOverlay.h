#pragma once
#include <DxLib.h>

class PauseOverlay {
public:
    // 初期化（選択項目をデフォルトに戻す）
    void Init();

    // 入力更新
    // - requestResume: true ならポーズ解除
    // - requestGoTitle: true ならタイトルへ戻る（ポーズ解除＋タイトル遷移）
    void Update(bool& requestResume, bool& requestGoTitle);

    // 描画
    void Draw();

private:
    int selectedIndex_ = 0; // 0=Resume, 1=Back to Title
    int blink_ = 0;
};