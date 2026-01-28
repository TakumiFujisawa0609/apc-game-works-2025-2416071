#pragma once
#include <DxLib.h>
#include <array>
#include <string>

// BGM種別
enum class BGM_ID {
    TITLE = 0,
    GAME,
    RESULT,
    MAX
};

// SE種別
enum class SE_ID {
    DECIDE = 0,  // 決定音（UI）
    BACK,        // 戻る音（UI）
    MOVE,        // UI移動
    JUMP,        // ジャンプ
    DASH,        // ダッシュ開始
    DEAD,        // 死亡時
    CHARGE_LOOP, // ため（チャージ）中のループSE
    MAX
};

class SoundManager {
public:
    static void CreateInstance();
    static SoundManager& GetInstance();

    void Init();
    void Destroy();

    // BGM
    void PlayBGM(BGM_ID id, bool loop = true);
    void StopBGM(BGM_ID id);
    void StopAllBGM();

    // SE（単発）
    void PlaySE(SE_ID id);

    // 追加: SE（ループ）
    void PlaySELoop(SE_ID id);
    void StopSE(SE_ID id);

    // 音量（0　255）
    void SetBGMVolume(int vol);
    void SetSEVolume(int vol);

    int GetBGMVolume() const { return bgmVolume_; }
    int GetSEVolume() const { return seVolume_; }

private:
    static SoundManager* instance_;

    std::array<int, static_cast<int>(BGM_ID::MAX)> bgmHandles_{};
    std::array<int, static_cast<int>(SE_ID::MAX)>  seHandles_{};

    int bgmVolume_ = 200;
    int seVolume_ = 220;

    void LoadAll();

    static inline bool IsValid(int handle) { return handle != -1; }
};