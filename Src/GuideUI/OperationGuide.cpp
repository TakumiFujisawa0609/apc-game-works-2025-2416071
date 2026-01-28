#include "OperationGuide.h"
#include "../Manager/SceneManager.h"
#include "../Application.h"

// シーン数（SceneManager::SCENE_ID に COUNT がある前提。無い場合は適宜修正）
static const int kSceneCount = static_cast<int>(SceneManager::SCENE_ID::COUNT);

// キャッシュ
static int gGuideImgs[/*kSceneCount*/64];           // 安全のため最大サイズ確保
static bool gInit = false;

// 画像パスのマッピング（実ファイルに合わせて調整）
static const char* gGuidePaths[/*kSceneCount*/64] = { 0 };

// 描画設定
static int gMargin = 24;     // 右下の余白
static double gScale = 1.0;  // 画像スケール

// 初期化ヘルパ
static void InitTablesOnce()
{
    if (gInit) return;
    gInit = true;

    // キャッシュ初期化
    for (int i = 0; i < 64; ++i) {
        gGuideImgs[i] = -1;
        gGuidePaths[i] = nullptr;
    }

    // シーン別の画像パス設定
    gGuidePaths[static_cast<int>(SceneManager::SCENE_ID::TITLE)]        = "Data/Image/Guide/guide_title.png";
    gGuidePaths[static_cast<int>(SceneManager::SCENE_ID::PLAYER_NUM)]   = "Data/Image/Guide/guide_playernum.png";
    gGuidePaths[static_cast<int>(SceneManager::SCENE_ID::GAME)]         = "Data/Image/Guide/guide_game.png";
    gGuidePaths[static_cast<int>(SceneManager::SCENE_ID::RESULT)]       = "Data/Image/Guide/guide_result.png";
}

void OperationGuide::EnsureLoaded(int sceneIndex)
{
    InitTablesOnce();

    if (sceneIndex < 0 || sceneIndex >= 64) return;
    if (gGuideImgs[sceneIndex] != -1) return;

    const char* path = gGuidePaths[sceneIndex];
    if (!path || path[0] == '\0') return;

    int h = LoadGraph(path);
    gGuideImgs[sceneIndex] = (h >= 0) ? h : -1;
}

void OperationGuide::DrawForScene(int sceneId)
{
    InitTablesOnce();

    // 範囲チェック
    if (sceneId < 0 || sceneId >= 64) return;

    // ロード確認
    EnsureLoaded(sceneId);
    int h = gGuideImgs[sceneId];
    if (h == -1) return;

    // 右下に描画（基準は画像左上。RotaGraph2で原点指定）
    const int x = Application::SCREEN_SIZE_X - gMargin;
    const int y = Application::SCREEN_SIZE_Y - gMargin;

    // 原点オフセットは 0,0（画像左上）。必要に応じて中央や右下原点に変更可能。
    DrawRotaGraph2(x, y, 0, 0, gScale, 0.0, h, TRUE, FALSE);
}

void OperationGuide::ReleaseAll()
{
    InitTablesOnce();

    for (int i = 0; i < 64; ++i) {
        if (gGuideImgs[i] != -1) {
            DeleteGraph(gGuideImgs[i]);
            gGuideImgs[i] = -1;
        }
    }
}

void OperationGuide::SetMargin(int px)
{
    gMargin = (px >= 0) ? px : 0;
}

void OperationGuide::SetScale(double s)
{
    if (s <= 0.0) return;
    gScale = s;
}
