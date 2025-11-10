#include "UIInput.h"
#include <DxLib.h>

static inline bool KeyTrg(int k) {
    return InputManager::GetInstance().IsTrgDown(k);
}

bool UIInput::IsPadConnected(InputManager::JOYPAD_NO no)
{
    return GetJoypadType(static_cast<int>(no)) != 0;
}

bool UIInput::PadBtn(InputManager::JOYPAD_NO no, InputManager::JOYPAD_BTN btn)
{
    return InputManager::GetInstance().IsPadBtnTrgDown(no, btn);
}

InputManager::JOYPAD_NO UIInput::ToPadNo(int idx)
{
    switch (idx)
    {
    case 0: return InputManager::JOYPAD_NO::PAD1;
    case 1: return InputManager::JOYPAD_NO::PAD2;
    case 2: return InputManager::JOYPAD_NO::PAD3;
    case 3: return InputManager::JOYPAD_NO::PAD4;
    default: return InputManager::JOYPAD_NO::PAD1;
    }
}

UINav UIInput::GetNavigate()
{
    UINav n;
    n.up = n.up || KeyTrg(KEY_INPUT_UP);
    n.down = n.down || KeyTrg(KEY_INPUT_DOWN);
    n.left = n.left || KeyTrg(KEY_INPUT_LEFT);
    n.right = n.right || KeyTrg(KEY_INPUT_RIGHT);
    n.ok = n.ok || KeyTrg(KEY_INPUT_RETURN) || KeyTrg(KEY_INPUT_SPACE);
    n.cancel = n.cancel || KeyTrg(KEY_INPUT_B);
    return n;
}

TitleInput UIInput::GetTitleInput()
{
    TitleInput ti;
    ti.anyPadConnected =
        IsPadConnected(InputManager::JOYPAD_NO::PAD1) ||
        IsPadConnected(InputManager::JOYPAD_NO::PAD2) ||
        IsPadConnected(InputManager::JOYPAD_NO::PAD3) ||
        IsPadConnected(InputManager::JOYPAD_NO::PAD4);

    ti.keyboardGoPlayerSelect = KeyTrg(KEY_INPUT_RETURN);
    ti.keyboardGoHowToPlay = KeyTrg(KEY_INPUT_SPACE);

    ti.goPlayerSelect = PadBtn(InputManager::JOYPAD_NO::PAD1, InputManager::JOYPAD_BTN::RIGHT); // B
    ti.goHowToPlay = PadBtn(InputManager::JOYPAD_NO::PAD1, InputManager::JOYPAD_BTN::DOWN);      // A
    return ti;
}

ManualInput UIInput::GetManualInput()
{
    ManualInput mi;
    mi.backToTitle = mi.backToTitle || KeyTrg(KEY_INPUT_SPACE);
    mi.goPlayerSelect = mi.goPlayerSelect || KeyTrg(KEY_INPUT_RETURN);
    mi.prevPage = mi.prevPage || KeyTrg(KEY_INPUT_LEFT);
    mi.nextPage = mi.nextPage || KeyTrg(KEY_INPUT_RIGHT);
    mi.backToTitle = mi.backToTitle || PadBtn(InputManager::JOYPAD_NO::PAD1, InputManager::JOYPAD_BTN::RIGHT);
    mi.goPlayerSelect = mi.goPlayerSelect || PadBtn(InputManager::JOYPAD_NO::PAD1, InputManager::JOYPAD_BTN::DOWN);
    mi.prevPage = mi.prevPage || PadBtn(InputManager::JOYPAD_NO::PAD1, InputManager::JOYPAD_BTN::LEFT);
    mi.nextPage = mi.nextPage || PadBtn(InputManager::JOYPAD_NO::PAD1, InputManager::JOYPAD_BTN::TOP);
    return mi;
}

PlayerNumInput UIInput::GetPlayerNumInput()
{
    PlayerNumInput pi;
    pi.pad1Connected = IsPadConnected(InputManager::JOYPAD_NO::PAD1);
    pi.left = pi.left || KeyTrg(KEY_INPUT_LEFT) || KeyTrg(KEY_INPUT_A) || KeyTrg(KEY_INPUT_UP) || KeyTrg(KEY_INPUT_W);
    pi.right = pi.right || KeyTrg(KEY_INPUT_RIGHT) || KeyTrg(KEY_INPUT_D) || KeyTrg(KEY_INPUT_DOWN) || KeyTrg(KEY_INPUT_S);
    pi.decide = pi.decide || KeyTrg(KEY_INPUT_RETURN) || KeyTrg(KEY_INPUT_SPACE);
    pi.back = pi.back || KeyTrg(KEY_INPUT_B);

    if (pi.pad1Connected)
    {
        pi.left = pi.left || PadBtn(InputManager::JOYPAD_NO::PAD1, InputManager::JOYPAD_BTN::LEFT);
        pi.right = pi.right || PadBtn(InputManager::JOYPAD_NO::PAD1, InputManager::JOYPAD_BTN::RIGHT);
        pi.decide = pi.decide || PadBtn(InputManager::JOYPAD_NO::PAD1, InputManager::JOYPAD_BTN::DOWN);
        pi.back = pi.back || PadBtn(InputManager::JOYPAD_NO::PAD1, InputManager::JOYPAD_BTN::TOP);
    }
    return pi;
}

CharacterSelectInput UIInput::GetCharacterSelectInput(int activePlayerIndex)
{
    CharacterSelectInput ci;

    // 有効PAD判定
    auto padNo = ToPadNo(activePlayerIndex);
    ci.activePadConnected = IsPadConnected(padNo);

    // キーボードは常に現在の順番プレイヤー操作として扱う
    ci.left = ci.left || KeyTrg(KEY_INPUT_LEFT) || KeyTrg(KEY_INPUT_A);
    ci.right = ci.right || KeyTrg(KEY_INPUT_RIGHT) || KeyTrg(KEY_INPUT_D);
    ci.decide = ci.decide || KeyTrg(KEY_INPUT_RETURN) || KeyTrg(KEY_INPUT_SPACE);
    ci.back = ci.back || KeyTrg(KEY_INPUT_B);

    // アクティブプレイヤーのPADのみ反応
    if (ci.activePadConnected)
    {
        ci.left = ci.left || PadBtn(padNo, InputManager::JOYPAD_BTN::LEFT);   // X
        ci.right = ci.right || PadBtn(padNo, InputManager::JOYPAD_BTN::RIGHT);  // B
        ci.decide = ci.decide || PadBtn(padNo, InputManager::JOYPAD_BTN::DOWN); // A
        ci.back = ci.back || PadBtn(padNo, InputManager::JOYPAD_BTN::TOP);      // Y
    }

    return ci;
}