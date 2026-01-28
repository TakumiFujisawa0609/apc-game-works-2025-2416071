#pragma once
#include <DxLib.h>

// 前方宣言（シーンID取得に使用）
namespace SceneManager { enum class SCENE_ID : int; }

class OperationGuide
{
public:
    // 指定シーンのガイド画像を描画（存在しない場合は何もしない）
    static void DrawForScene(int sceneId);

    // 便宜用：SceneManager::SCENE_ID をそのまま渡せるオーバーロード
    static void DrawForScene(SceneManager::SCENE_ID sceneId) {
        DrawForScene(static_cast<int>(sceneId));
    }

    // 全ガイド画像の破棄（アプリ終了時などで呼ぶ）
    static void ReleaseAll();

    // 表示位置とスケールの調整（必要に応じて呼び出し）
    static void SetMargin(int px);
    static void SetScale(double s);

private:
    static void EnsureLoaded(int sceneIndex);
};

