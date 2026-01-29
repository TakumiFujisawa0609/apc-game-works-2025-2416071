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

    // シングルプレイモード（プレイヤー数が1のとき true）
    bool singlePlayerMode_ = false;

    // シングルプレイ時の人間プレイヤーID（0固定）
    int humanPlayerId_ = 0;

    // 「YOU」マーカーの点滅用タイマー
    int youMarkerTimer_ = 0;
    static constexpr int YOU_MARKER_HIGHLIGHT_FRAMES = 240; // 約4秒

    // 3Dの頭上マーカー（YOU/CPU）描画
    void DrawPlayerMarkers3D();

    // HUDの凡例（必要に応じて使用）
    void DrawPlayerHUDLegend();

    // ワールド座標に対して2Dラベルを描画
    void DrawLabelAtWorld(const char* text, const VECTOR& worldPos, unsigned int color);

    // 追加: プレイヤーの生存状態を左上に表示するHUD
    void DrawPlayerStatusHUD();

    // 操作説明用画像ボタン変数
	int attackButtonImg_ = -1;

	// 移動操作・突進方向決定用のボタン画像
	int moveButtonImg_ = -1;

    // ボタン表示用フラグ
	bool showButtonHints_ = true;

    // SE状態管理
    std::vector<bool> prevCharging_;
    std::vector<bool> prevDashing_;


};