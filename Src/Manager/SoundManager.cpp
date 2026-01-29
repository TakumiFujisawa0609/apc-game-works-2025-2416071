#include "SoundManager.h"
#include <vector>

SoundManager* SoundManager::instance_ = nullptr;

void SoundManager::CreateInstance() {
    if (!instance_) instance_ = new SoundManager();
}

SoundManager& SoundManager::GetInstance() {
    if (!instance_) CreateInstance();
    return *instance_;
}

void SoundManager::Init() {
    for (auto& h : bgmHandles_) h = -1;
    for (auto& h : seHandles_)  h = -1;

    LoadAll();

    for (auto h : bgmHandles_) { if (IsValid(h)) ChangeVolumeSoundMem(bgmVolume_, h); }
    for (auto h : seHandles_) { if (IsValid(h)) ChangeVolumeSoundMem(seVolume_, h); }
}

void SoundManager::Destroy() {
    for (auto& h : bgmHandles_) { if (IsValid(h)) { DeleteSoundMem(h); h = -1; } }
    for (auto& h : seHandles_) { if (IsValid(h)) { DeleteSoundMem(h); h = -1; } }
    delete instance_;
    instance_ = nullptr;
}

void SoundManager::LoadAll() {
    // BGMパス（必要に応じて差し替え）
    const std::vector<std::string> bgmPaths = {
        "Data/Sound/BGM/title.wav",
        "Data/Sound/BGM/game.wav",
        "Data/Sound/BGM/result.wav"
    };
    for (int i = 0; i < static_cast<int>(BGM_ID::MAX) && i < (int)bgmPaths.size(); ++i) {
        int h = LoadSoundMem(bgmPaths[i].c_str());
        bgmHandles_[i] = h;
        if (IsValid(h)) ChangeVolumeSoundMem(bgmVolume_, h);
    }

    // SEパス（必要に応じて差し替え）
    const std::vector<std::string> sePaths = {
        "Data/Sound/SE/GoNextScene.mp3",     // DECIDE
        "Data/Sound/SE/back.wav",       // BACK
        "Data/Sound/SE/Select_Hb.mp3",       // MOVE
		"Data/Sound/SE/Select_Hb2.mp3",      // MOVE2
        "Data/Sound/SE/jump.wav",       // JUMP
        "Data/Sound/SE/DASH.mp3",       // DASH
        "Data/Sound/SE/dead.wav",       // DEAD
        "Data/Sound/SE/Ch_loop.mp3" // CHARGE_LOOP
    };
    for (int i = 0; i < static_cast<int>(SE_ID::MAX) && i < (int)sePaths.size(); ++i) {
        int h = LoadSoundMem(sePaths[i].c_str());
        seHandles_[i] = h;
        if (IsValid(h)) ChangeVolumeSoundMem(seVolume_, h);
    }
}

void SoundManager::PlayBGM(BGM_ID id, bool loop) {
    int idx = static_cast<int>(id);
    if (idx < 0 || idx >= static_cast<int>(BGM_ID::MAX)) return;
    int h = bgmHandles_[idx];
    if (!IsValid(h)) return;
    if (CheckSoundMem(h) == 1) return; // すでに再生中ならスキップ
    ChangeVolumeSoundMem(bgmVolume_, h);
    PlaySoundMem(h, loop ? DX_PLAYTYPE_LOOP : DX_PLAYTYPE_BACK);
}

void SoundManager::StopBGM(BGM_ID id) {
    int idx = static_cast<int>(id);
    if (idx < 0 || idx >= static_cast<int>(BGM_ID::MAX)) return;
    int h = bgmHandles_[idx];
    if (!IsValid(h)) return;
    StopSoundMem(h);
}

void SoundManager::StopAllBGM() {
    for (auto h : bgmHandles_) {
        if (IsValid(h)) StopSoundMem(h);
    }
}

void SoundManager::PlaySE(SE_ID id) {
    int idx = static_cast<int>(id);
    if (idx < 0 || idx >= static_cast<int>(SE_ID::MAX)) return;
    int h = seHandles_[idx];
    if (!IsValid(h)) return;
    ChangeVolumeSoundMem(seVolume_, h);
    PlaySoundMem(h, DX_PLAYTYPE_BACK); // 単発
}

void SoundManager::PlaySELoop(SE_ID id) {
    int idx = static_cast<int>(id);
    if (idx < 0 || idx >= static_cast<int>(SE_ID::MAX)) return;
    int h = seHandles_[idx];
    if (!IsValid(h)) return;
    // ループ再生、すでに再生中なら重複再生しない
    if (CheckSoundMem(h) != 1) {
        ChangeVolumeSoundMem(seVolume_, h);
        PlaySoundMem(h, DX_PLAYTYPE_LOOP);
    }
}

void SoundManager::StopSE(SE_ID id) {
    int idx = static_cast<int>(id);
    if (idx < 0 || idx >= static_cast<int>(SE_ID::MAX)) return;
    int h = seHandles_[idx];
    if (!IsValid(h)) return;
    StopSoundMem(h);
}

void SoundManager::SetBGMVolume(int vol) {
    if (vol < 0) vol = 0; if (vol > 255) vol = 255;
    bgmVolume_ = vol;
    for (auto h : bgmHandles_) { if (IsValid(h)) ChangeVolumeSoundMem(bgmVolume_, h); }
}

void SoundManager::SetSEVolume(int vol) {
    if (vol < 0) vol = 0; if (vol > 255) vol = 255;
    seVolume_ = vol;
    for (auto h : seHandles_) { if (IsValid(h)) ChangeVolumeSoundMem(seVolume_, h); }
}