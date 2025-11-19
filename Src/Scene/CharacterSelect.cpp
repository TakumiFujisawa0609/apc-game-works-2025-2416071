#include "CharacterSelect.h"
#include "../Object/UIInput.h"
#include "../Manager/SceneManager.h"

// キャラクターごとの色定義
static const unsigned int CHARACTER_COLORS[] = {
	GetColor(255, 80, 80),   // 1P: 赤
	GetColor(80, 80, 255),   // 2P: 青
	GetColor(80, 255, 80),   // 3P: 緑
	GetColor(80, 80, 80),  // 4P: 黒
	// 必要に応じて追加
};

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

void CharacterSelect::Init()
{
	// 人数取得 (引数 0 なら SceneManager から)
	if (playerCount_ <= 0)
	{
		int num = 1;
		try { num = SceneManager::GetInstance().GetPlayerNum(); }
		catch (...) { num = 1; }
		playerCount_ = num;
	}
	if (playerCount_ < 1) playerCount_ = 1;
	if (playerCount_ > MAX_PLAYERS) playerCount_ = MAX_PLAYERS;

	// キャラ名準備
	characterNames_.clear();
	for (int i = 0; i < MAX_CHARACTERS; ++i)
	{
		characterNames_.push_back("Character " + std::to_string(i + 1));
	}

	// 最低限、キャラ数 < 人数 の場合は後続で詰むため補正
	if (static_cast<int>(characterNames_.size()) < playerCount_)
	{
		// 足りない分をダミー追加
		for (int i = static_cast<int>(characterNames_.size()); i < playerCount_; ++i)
		{
			characterNames_.push_back("Character " + std::to_string(i + 1));
		}
	}

	selectedIndex_.assign(playerCount_, 0);
	confirmed_.assign(playerCount_, false);

	// 先頭から重複しない初期値 (0,1,2,...)
	for (int p = 0; p < playerCount_; ++p)
	{
		selectedIndex_[p] = p % static_cast<int>(characterNames_.size());
	}

	// もし、遊ぶ人数が1人だけなら1人プレイモードに設定


	currentPlayer_ = 0;
	finished_ = false;
	duplicateBlock_ = false;
}

void CharacterSelect::Update()
{
	if (finished_)
	{
		ApplySelection();
		SceneManager::GetInstance().ChangeScene(SceneManager::SCENE_ID::GAME);
		return;
	}

	HandleInput();
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

	// 未確定ならキャラ変更 (重複回避)
	if (!confirmed_[currentPlayer_])
	{
		if (in.left)  ChangeCharacter(-1);
		if (in.right) ChangeCharacter(+1);
	}

	// 決定 (重複している場合は ChangeCharacter で回避しているので基本成功)
	if (in.decide && !confirmed_[currentPlayer_])
	{
		// セーフティ (万一重複なら確定不可)
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
	int size = static_cast<int>(characterNames_.size());
	if (size <= 0) return;

	int current = selectedIndex_[currentPlayer_];
	int dir = (delta < 0) ? -1 : 1;

	// 探索開始位置
	int start = (current + dir + size) % size;
	int candidate = FindNextAvailable(start, dir);

	if (candidate == current)
	{
		// 全て埋まっている (理論上 playerCount_ <= size なら起きない)
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
		{
			return true;
		}
	}
	return false;
}

int CharacterSelect::FindNextAvailable(int start, int dir) const
{
	int size = static_cast<int>(characterNames_.size());
	int current = selectedIndex_[currentPlayer_];

	// 1周分探索
	for (int i = 0; i < size; ++i)
	{
		int idx = (start + (dir * i) + size) % size;
		if (idx == current) continue; // 現在選択はスキップ (別候補を探す)
		if (!IsTaken(idx))
		{
			return idx;
		}
	}
	// 見つからない場合は現状維持
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
	confirmed_[currentPlayer_] = false; // 再選択可能
}

void CharacterSelect::CompleteSelection()
{
	finished_ = true;
}

void CharacterSelect::Draw()
{
	DrawFormatString2(60, 40, GetColor(255, 255, 255), -1, "キャラクター選択 (重複禁止 / 順番制)");

	DrawPadWarning();
	DrawPlayerEntries();
	DrawGuide();
	DrawDuplicateNotice();
}

void CharacterSelect::DrawPadWarning()
{
	auto in = UIInput::GetCharacterSelectInput(currentPlayer_);
	if (!in.activePadConnected)
	{
		DrawFormatString2(60, 70, GetColor(255, 180, 120), -1,
			"注意: 現在の手番 P%d の PAD 未接続。キーボード操作可能。",
			currentPlayer_ + 1);
	}
}

void CharacterSelect::DrawPlayerEntries()
{
	int y = 120;
	for (int i = 0; i < playerCount_; ++i)
	{
		bool isActive = (i == currentPlayer_);
		bool ok = confirmed_[i];
		int sel = selectedIndex_[i];
		if (sel < 0 || sel >= static_cast<int>(characterNames_.size())) sel = 0;

		// プレイヤー番号で色を固定
		unsigned int charColor = CHARACTER_COLORS[i % (sizeof(CHARACTER_COLORS) / sizeof(CHARACTER_COLORS[0]))];

		unsigned int col =
			isActive ? GetColor(255, 255, 0) :
			ok ? GetColor(120, 255, 120) :
			GetColor(255, 255, 255);

		// キャラクター名の横に色付き四角を表示
		DrawBox(30, y, 54, y + 24, charColor, TRUE);

		DrawFormatString2(60, y, col, -1, "%s P%d : %s %s",
			isActive ? ">" : " ",
			i + 1,
			characterNames_[sel].c_str(),
			ok ? "[OK]" : "");
		y += 32;
	}

	// 操作するプレイヤーの色を覚えてくださいと表示
	DrawFormatString2(60, y + 20, GetColor(200, 200, 255), -1,
		"※ 操作するプレイヤーの色を覚えてください。");
}

void CharacterSelect::DrawGuide()
{
	DrawFormatString2(440, 120, GetColor(180, 180, 255), -1, "キャラ変更: ←/A / →/D / (PAD: X/B) 重複は自動スキップ");
	DrawFormatString2(440, 152, GetColor(180, 180, 255), -1, "決定: Enter / Space / (PAD: A)");
	DrawFormatString2(440, 184, GetColor(180, 180, 255), -1, "戻る: B / (PAD: Y) 前プレイヤーへ");
	DrawFormatString2(440, 216, GetColor(180, 180, 255), -1, "黄=選択中 / 緑=確定済み");
}

void CharacterSelect::DrawDuplicateNotice()
{
	int cnt_ = 0;
	if (duplicateBlock_)
	{
		
		DrawFormatString2(60, 300, GetColor(255, 100, 100), -1,
			"このキャラは既に他プレイヤーが確定しています。別キャラを選択してください。");
		cnt_++;
		if (cnt_ == 360) cnt_ = 0;
	}
}

void CharacterSelect::ApplySelection()
{
	// 選択結果を SceneManager に反映 (重複はここまでで存在しない前提)
	std::vector<int> types;
	types.reserve(playerCount_);
	for (int p = 0; p < playerCount_; ++p)
	{
		int idx = selectedIndex_[p];
		if (idx < 0) idx = 0;
		if (idx >= static_cast<int>(characterNames_.size())) idx = static_cast<int>(characterNames_.size()) - 1;
		types.push_back(idx);
	}
	SceneManager::GetInstance().SetSelectedPlayerNums(types);
}

void CharacterSelect::Release()
{
	characterNames_.clear();
	selectedIndex_.clear();
	confirmed_.clear();
}