#include "PauseOverlay.h"
#include "../Application.h"
#include "../Manager/InputManager.h"

void PauseOverlay::Init() {
    selectedIndex_ = 0;
    blink_ = 0;
}

void PauseOverlay::Update(bool& requestResume, bool& requestGoTitle) {
    requestResume = false;
    requestGoTitle = false;

    auto& ins = InputManager::GetInstance();

    // ナビゲーション
    bool up = ins.IsTrgDown(KEY_INPUT_UP) ||
        ins.IsPadBtnTrgDown(InputManager::JOYPAD_NO::PAD1, InputManager::JOYPAD_BTN::D_PAD_UP);
    bool down = ins.IsTrgDown(KEY_INPUT_DOWN) ||
        ins.IsPadBtnTrgDown(InputManager::JOYPAD_NO::PAD1, InputManager::JOYPAD_BTN::D_PAD_DOWN);

    if (up) { selectedIndex_ = (selectedIndex_ + 1 + 2) % 2; }   // 循環
    if (down) { selectedIndex_ = (selectedIndex_ + 1) % 2; }

    // 決定/キャンセル
    bool confirm = ins.IsTrgDown(KEY_INPUT_RETURN) || ins.IsTrgDown(KEY_INPUT_SPACE) ||
        ins.IsPadBtnTrgDown(InputManager::JOYPAD_NO::PAD1, InputManager::JOYPAD_BTN::DOWN);
    bool cancel = ins.IsTrgDown(KEY_INPUT_ESCAPE) ||
        ins.IsPadBtnTrgDown(InputManager::JOYPAD_NO::PAD1, InputManager::JOYPAD_BTN::RIGHT);

    if (cancel) {
        // ESC もう一度で解除
        requestResume = true;
        return;
    }
    if (confirm) {
        if (selectedIndex_ == 0) {
            requestResume = true;
        }
        else {
            requestGoTitle = true;
        }
        return;
    }

    blink_ = (blink_ + 1) % 60;
}

void PauseOverlay::Draw() {
    // 画面全体を半透明で暗くする
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 160);
    DrawBox(0, 0, Application::SCREEN_SIZE_X, Application::SCREEN_SIZE_Y, GetColor(0, 0, 0), TRUE);
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

    // ポップアップ枠
    const int w = 420, h = 220;
    const int x = Application::SCREEN_SIZE_X / 2 - w / 2;
    const int y = Application::SCREEN_SIZE_Y / 2 - h / 2;

    DrawBox(x, y, x + w, y + h, GetColor(40, 40, 60), TRUE);
    DrawBox(x, y, x + w, y + h, GetColor(220, 220, 255), FALSE);

    // タイトル
    DrawFormatString(x + 24, y + 20, GetColor(255, 255, 255), "Paused");

    // メニュー項目
    const char* items[2] = { "Resume", "Back to Title" };
    for (int i = 0; i < 2; ++i) {
        int iy = y + 70 + i * 40;
        bool sel = (i == selectedIndex_);
        unsigned int col = sel ? GetColor(255, 255, 120) : GetColor(220, 220, 220);
        DrawFormatString(x + 40, iy, col, "%s", items[i]);

        if (sel) {
            int blink = (blink_ / 8) % 2;
            unsigned int hl = blink ? GetColor(255, 255, 120) : GetColor(255, 220, 80);
            DrawBox(x + 28, iy - 6, x + w - 28, iy + 20, hl, FALSE);
        }
    }

    // ヒント
    DrawFormatString(x + 24, y + h - 32, GetColor(180, 200, 255),
        "Enter: Confirm   Esc: Close   D-Pad: Up/Down");
}