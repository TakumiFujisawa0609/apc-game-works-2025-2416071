#include "CharacterSelect.h"
#include "../Object/UIInput.h"
#include "../Manager/SceneManager.h"
#include "../Object/Player/Common/PlayerManager.h"
#include "../Utility/AsoUtility.h"

// 簡易アルファ混合（暗くする）
static unsigned int ColorMul(unsigned int c, float a)
{
    if (a < 0.0f) a = 0.0f;
    if (a > 1.0f) a = 1.0f;

    int r = (int)(c & 0xFF);
    int g = (int)((c >> 8) & 0xFF);
    int b = (int)((c >> 16) & 0xFF);

    r = (int)(r * a);
    g = (int)(g * a);
    b = (int)(b * a);

    return GetColor(r, g, b);
}

CharacterSelect::CharacterSelect(int playerCount)
    : playerCount_(playerCount)
    , currentPlayer_(0)
    , finished_(false)
    , duplicateBlock_(false)
{
}

CharacterSelect::~CharacterSelect()
{
}

void CharacterSelect::BuildDefaultCharacters()
{
    characterNames_.clear();
    characterNames_.push_back("Type A");
    characterNames_.push_back("Type B");
    characterNames_.push_back("Type C");
    characterNames_.push_back("Type D");
}

void CharacterSelect::Init()
{
    // プレイヤー数の取得
    if (playerCount_ <= 0)
    {
        int num = 1;
        try { num = SceneManager::GetInstance().GetPlayerNum(); }
        catch (...) { num = 1; }
        playerCount_ = num;
    }
    if (playerCount_ < 1) playerCount_ = 1;
    if (playerCount_ > MAX_PLAYERS) playerCount_ = MAX_PLAYERS;

    BuildDefaultCharacters();

    selectedIndex_.assign(playerCount_, 0);
    confirmed_.assign(playerCount_, false);

    for (int p = 0; p < playerCount_; ++p) {
        selectedIndex_[p] = p % (int)characterNames_.size();
    }

    // 3Dプレビュー用モデル読み込み
    for (int i = 0; i < MAX_CHARACTERS; ++i)
    {
        const char* path = PlayerManager::GetModelPathForType(static_cast<PlayerType>(i));
        if (path) {
            previewModelId_[i] = MV1LoadModel(path);
        }
        else {
            previewModelId_[i] = -1;
        }
    }

    currentPlayer_ = 0;
    finished_ = false;
    duplicateBlock_ = false;
    blinkCounter_ = 0;
}

void CharacterSelect::Update()
{
    if (finished_)
    {
        // 選択結果を SceneManager に適用し、GAMEへ
        std::vector<int> types;
        types.reserve(playerCount_);
        for (int p = 0; p < playerCount_; ++p) {
            int idx = selectedIndex_[p];
            if (idx < 0) idx = 0;
            if (idx >= (int)characterNames_.size()) idx = (int)characterNames_.size() - 1;
            types.push_back(idx);
        }
        SceneManager::GetInstance().SetSelectedPlayerNums(types);
        SceneManager::GetInstance().ChangeScene(SceneManager::SCENE_ID::GAME);
        return;
    }

    HandleInput();
    blinkCounter_ = (blinkCounter_ + 1) % 60;
}

void CharacterSelect::HandleInput()
{
    auto in = UIInput::GetCharacterSelectInput(currentPlayer_);

    // 戻る
    if (in.back)
    {
        BackToPrevious();
        return;
    }

    duplicateBlock_ = false;

    // 非確定時のみキャラ移動
    if (!confirmed_[currentPlayer_])
    {
        if (in.left)  ChangeCharacter(-1);
        if (in.right) ChangeCharacter(+1);
    }

    // 決定
    if (in.decide && !confirmed_[currentPlayer_])
    {
        if (IsTaken(selectedIndex_[currentPlayer_]))
        {
            duplicateBlock_ = true;
            return;
        }
        DecideCurrent();
        return;
    }
}

void CharacterSelect::ChangeCharacter(int delta)
{
    int size = (int)characterNames_.size();
    if (size <= 0) return;

    int current = selectedIndex_[currentPlayer_];
    int dir = (delta < 0) ? -1 : 1;

    int start = (current + dir + size) % size;
    int candidate = FindNextAvailable(start, dir);

    if (candidate == current)
    {
        duplicateBlock_ = IsTaken(current);
        return;
    }

    selectedIndex_[currentPlayer_] = candidate;
    duplicateBlock_ = false;
}

bool CharacterSelect::IsTaken(int characterIdx) const
{
    for (int p = 0; p < playerCount_; ++p)
    {
        if (p == currentPlayer_) continue;
        if (confirmed_[p] && selectedIndex_[p] == characterIdx)
            return true;
    }
    return false;
}

int CharacterSelect::FindNextAvailable(int start, int dir) const
{
    int size = (int)characterNames_.size();
    int current = selectedIndex_[currentPlayer_];

    for (int i = 0; i < size; ++i)
    {
        int idx = (start + (dir * i) + size) % size;
        if (idx == current) continue;
        if (!IsTaken(idx))
            return idx;
    }
    return current;
}

void CharacterSelect::DecideCurrent()
{
    confirmed_[currentPlayer_] = true;
    AdvancePlayer();
}

void CharacterSelect::BackToPrevious()
{
    if (currentPlayer_ == 0)
    {
        SceneManager::GetInstance().ChangeScene(SceneManager::SCENE_ID::PLAYERNUMBERSELECT);
        return;
    }
    RevertPlayer();
}

void CharacterSelect::AdvancePlayer()
{
    currentPlayer_++;
    if (currentPlayer_ >= playerCount_)
    {
        CompleteSelection();
    }
}

void CharacterSelect::RevertPlayer()
{
    currentPlayer_--;
    if (currentPlayer_ < 0) currentPlayer_ = 0;
    confirmed_[currentPlayer_] = false;
}

void CharacterSelect::CompleteSelection()
{
    finished_ = true;
    // 保険：完了直後の描画で out of range にならないようクランプ
    if (currentPlayer_ >= playerCount_) currentPlayer_ = playerCount_ - 1;
    if (currentPlayer_ < 0) currentPlayer_ = 0;
}

void CharacterSelect::Draw()
{
    DrawHeader();
    DrawCharacterGrid();
    DrawPlayerList();

    // 3Dプレビュー＋パラメータ（固定表示）
    DrawPreviewModelAndParams();

    DrawFooter();
    DrawDuplicateNotice();
}

int CharacterSelect::ActivePlayerIndexForDraw() const
{
    if (playerCount_ <= 0) return 0;
    int idx = currentPlayer_;
    if (idx >= playerCount_) idx = playerCount_ - 1;
    if (idx < 0) idx = 0;
    return idx;
}

void CharacterSelect::DrawHeader()
{
    const int activeIdx = ActivePlayerIndexForDraw();
    DrawFormatString(60, 40, GetColor(255, 255, 255), "キャラクター選択（順番選択）");
    if (!finished_) {
        DrawFormatString(60, 66, GetColor(180, 180, 255), "P%d が選択中", activeIdx + 1);
    }
    else {
        DrawFormatString(60, 66, GetColor(180, 255, 180), "選択完了中…");
    }
}

void CharacterSelect::DrawCharacterGrid()
{
    const int activeIdx = ActivePlayerIndexForDraw();

    int idx = 0;
    for (int r = 0; r < gridRows_; ++r)
    {
        for (int c = 0; c < gridCols_; ++c)
        {
            if (idx >= (int)characterNames_.size()) return;

            int x = gridLeft_ + c * (cardW_ + gridGapX_);
            int y = gridTop_ + r * (cardH_ + gridGapY_);

            // ハイライトは finished_ で無効化。参照は必ず activeIdx を使う
            bool highlight = (!finished_) && (idx == selectedIndex_[activeIdx]);
            bool taken = IsTaken(idx);

            DrawCharacterCard(idx, x, y, cardW_, cardH_, highlight, taken);
            ++idx;
        }
    }
}

void CharacterSelect::DrawCharacterCard(int idx, int x, int y, int w, int h, bool highlight, bool taken)
{
    // 背景色は固定（必要なら PlayerType で変更）
    unsigned int bg = GetColor(60, 60, 60);
    if (taken) bg = ColorMul(bg, 0.5f);

    DrawBox(x, y, x + w, y + h, bg, TRUE);
    DrawBox(x, y, x + w, y + h, GetColor(20, 20, 20), FALSE);

    const char* name = characterNames_[idx].c_str();
    DrawFormatString(x + 10, y + 8, GetColor(255, 255, 255), "%s", name);

    // ハイライト
    if (highlight)
    {
        int blink = (blinkCounter_ / 10) % 2;
        unsigned int hl = blink ? GetColor(255, 255, 120) : GetColor(255, 200, 60);
        for (int t = 0; t < 3; ++t) {
            DrawBox(x - t, y - t, x + w + t, y + h + t, hl, FALSE);
        }
    }
}

void CharacterSelect::DrawPlayerList()
{
    const int activeIdx = ActivePlayerIndexForDraw();

    int x = rightPanelX_;
    int y = rightPanelY_;

    DrawFormatString(x, y - 24, GetColor(200, 200, 255), "プレイヤー状態");
    for (int i = 0; i < playerCount_; ++i)
    {
        bool isActive = (!finished_) && (i == activeIdx);
        bool ok = confirmed_[i];
        int sel = selectedIndex_[i];
        if (sel < 0 || sel >= (int)characterNames_.size()) sel = 0;

        unsigned int nameCol =
            isActive ? GetColor(255, 255, 100) :
            ok ? GetColor(120, 255, 120) :
            GetColor(255, 255, 255);

        DrawFormatString(x, y, nameCol, "P%d : %s %s",
            i + 1,
            characterNames_[sel].c_str(),
            ok ? "[OK]" : "");

        y += rightLineH_;
    }
}

void CharacterSelect::DrawPreviewModelAndParams()
{
    const int activeIdx = ActivePlayerIndexForDraw();

    // 選択中のキャラは activeIdx で取得（currentPlayer_ を使わない）
    int sel = selectedIndex_[activeIdx];
    if (sel < 0 || sel >= MAX_CHARACTERS) sel = 0;

    // プレビュー用カメラ（固定）
    {
        VECTOR camPos = VGet(0.0f, 250.0f, -800.0f);
        float pitch = 20.0f * DX_PI_F / 180.0f;
        SetCameraPositionAndAngle(camPos, pitch, 0.0f, 0.0f);
    }

    // モデル描画（固定）
    int modelId = previewModelId_[sel];
    if (modelId != -1)
    {
        MV1SetRotationXYZ(modelId, VGet(0.0f, 0.0f, 0.0f));
        MV1SetScale(modelId, VGet(previewScale_, previewScale_, previewScale_));
        MV1SetPosition(modelId, VGet(previewModelOffsetX_, 0.0f, 0.0f));
        MV1DrawModel(modelId);
    }

    // パラメータ棒グラフ
    PlayerParam p = PlayerManager::GetDefaultParamForType(static_cast<PlayerType>(sel));
    auto clamp01 = [](float v) { return v < 0.0f ? 0.0f : (v > 1.0f ? 1.0f : v); };
    float speedT = clamp01(p.speed / 10.0f + 0.1f);
    float jumpT = clamp01(p.jumpPower / 10.0f + 0.1f);
    float weightT = clamp01(p.weight / 15.0f);

    int bx = previewAreaX_ - 180;
    int by = previewAreaY_ + 120;
    int bw = 360;

    auto drawBar = [&](const char* label, float t, int y, unsigned int col) {
        int filled = (int)(bw * t);
        DrawFormatString(bx, y - 14, GetColor(230, 230, 230), "%s", label);
        DrawBox(bx, y, bx + bw, y + 14, GetColor(60, 60, 60), TRUE);
        DrawBox(bx, y, bx + filled, y + 14, col, TRUE);
        DrawBox(bx, y, bx + bw, y + 14, GetColor(20, 20, 20), FALSE);
        };
    drawBar("Speed", speedT, by + 0, GetColor(120, 220, 255));
    drawBar("Jump ", jumpT, by + 24, GetColor(120, 255, 160));
    drawBar("Weight", weightT, by + 48, GetColor(255, 180, 120));
}

void CharacterSelect::DrawFooter()
{
    int baseY = 420;
    DrawFormatString(60, baseY + 0, GetColor(180, 180, 255), "操作ガイド: ←/→ で選択  Enter/Space で決定  B で戻る");
    DrawFormatString(60, baseY + 20, GetColor(160, 160, 160), "プレビューは固定表示（プレイヤーは移動しません）。");
}

void CharacterSelect::DrawDuplicateNotice()
{
    if (!duplicateBlock_) return;

    int blink = (blinkCounter_ / 8) % 2;
    if (blink == 0) return;

    DrawFormatString(60, 360, GetColor(255, 120, 120),
        "このキャラは既に他のプレイヤーが選択済みです。別のキャラを選択してください。");
}

void CharacterSelect::Release()
{
    // 3Dモデル解放
    for (int i = 0; i < MAX_CHARACTERS; ++i)
    {
        if (previewModelId_[i] != -1)
        {
            MV1DeleteModel(previewModelId_[i]);
            previewModelId_[i] = -1;
        }
    }

    characterNames_.clear();
    selectedIndex_.clear();
    confirmed_.clear();
}

