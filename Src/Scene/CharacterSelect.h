#pragma once
#include "SceneBase.h"
#include <vector>
#include <string>
#include <DxLib.h>

// キャラクター選択シーン（順番選択: P1→P2→…）
class CharacterSelect : public SceneBase
{
public:
    static constexpr int MAX_PLAYERS = 4;
    static constexpr int MAX_CHARACTERS = 4;

    CharacterSelect(int playerCount = 0);
    ~CharacterSelect() override;

    void Init() override;
    void Update() override;
    void Draw() override;
    void Release() override;

    bool IsFinished() const { return finished_; }

private:
    // 入力処理
    void HandleInput();
    void ChangeCharacter(int delta);
    void DecideCurrent();
    void BackToPrevious();

    // 順番管理
    void AdvancePlayer();
    void RevertPlayer();
    void CompleteSelection();

    // 重複チェック
    bool IsTaken(int characterIdx) const;
    int  FindNextAvailable(int start, int dir) const;

    // 描画
    int ActivePlayerIndexForDraw() const;
    void DrawHeader();
    void DrawCharacterGrid();
    void DrawCharacterCard(int idx, int x, int y, int w, int h, bool highlight, bool taken);
    void DrawPlayerList();
    void DrawFooter();
    void DrawDuplicateNotice();

    // 3Dプレビュー＋パラメータ表示
    void DrawPreviewModelAndParams();

    // 追加: カード内パラメータ表示
    void DrawCardStatsBars(const struct PlayerParam& p, int x, int y, int w, int h);

    // 見た目設定
    void BuildDefaultCharacters();

    // レイアウト
    int gridCols_ = 2;
    int gridRows_ = 2;
    int cardW_ = 240;
    int cardH_ = 140;
    int gridLeft_ = 60;
    int gridTop_ = 100;
    int gridGapX_ = 40;
    int gridGapY_ = 40;

    int rightPanelX_ = 420;
    int rightPanelY_ = 100;
    int rightLineH_ = 30;

    // プレビュー表示設定
    int previewAreaX_ = 900;
    int previewAreaY_ = 260;
    float previewScale_ = 1.4f;
    float previewModelOffsetX_ = 400.0f;

    // 状態
    int playerCount_;
    int currentPlayer_;
    bool finished_;
    bool duplicateBlock_;

    std::vector<std::string> characterNames_;
    std::vector<int>  selectedIndex_;
    std::vector<bool> confirmed_;

    // 3Dプレビュー用モデルハンドル
    int previewModelId_[MAX_CHARACTERS] = { -1, -1, -1, -1 };

    // 背景
	int bgImageId_ = -1;

    // 点滅
    int blinkCounter_ = 0;

    // 操作ガイド
	int cursorLImg_ = -1;

    // 確定ボタン画像
	int decideBtnImg_ = -1;
};