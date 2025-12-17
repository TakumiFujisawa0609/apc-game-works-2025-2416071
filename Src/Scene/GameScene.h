#pragma once
#include "SceneBase.h"
#include "../Object/Stage/Stage.h" 

class Grid;
class StageManager;
class PlayerManager;
class Stage;
class Player_1;

class GameScene : public SceneBase
{
public:
    GameScene();
    ~GameScene(void)override;

    void Init(void)override;
    void Update(void)override;
    void Draw(void)override;
    void Draw3D(void);
    void Release(void)override;

    int GetPlayerNum(void) const { return playerNum_; }

private:
    int playerNum_ = 1;
    PlayerManager* playerManager_ = nullptr;
    Stage& stage_ = Stage::GetInstance();
    Player_1* debugPlayer_ = nullptr;
    int transitionTimer_ = 0;

    bool singlePlayerMode_ = false;

    // 人間（1人プレイ時はID=0）
    int humanPlayerId_ = 0;

    // 開始直後強調用
    int youMarkerTimer_ = 0;
    static constexpr int YOU_MARKER_HIGHLIGHT_FRAMES = 240; // 約4秒

    // 3Dマーカー描画（YOU/CPU ラベル）
    void DrawPlayerMarkers3D();

    // 左上のHUD
    void DrawPlayerHUDLegend();

    // ワールド座標→スクリーン座標でラベル描画
    void DrawLabelAtWorld(const char* text, const VECTOR& worldPos, unsigned int color);
};