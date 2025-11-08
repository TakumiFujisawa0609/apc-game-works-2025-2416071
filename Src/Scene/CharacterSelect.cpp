#include "CharacterSelect.h"
#include "../Object/Player/Common/PlayerManager.h"
#include "../Object/Player/Common/Player.h"
#include "../Manager/SceneManager.h"
#include <algorithm>

// 常に用意するタイプ数
static const int TYPE_COUNT = 4;

CharacterSelect::CharacterSelect(int playerCount)
	: currentPlayer_(0),
	finished_(false),
	playerCount_(playerCount)
{
	// 引数が未指定または範囲外なら SceneManager から人数を取得
	if (playerCount_ <= 0 || playerCount_ > MAX_PLAYERS)
	{
		// SceneManager は PlayerNumScene で 0-based の selectNum_ を保存しているため +1
		int smVal = SceneManager::GetInstance().GetPlayerNum();
		playerCount_ = smVal + 1;
	}

	// 安全に 1..MAX_PLAYERS に丸める
	if (playerCount_ < 1) playerCount_ = 1;
	if (playerCount_ > MAX_PLAYERS) playerCount_ = MAX_PLAYERS;

	Init();
}

CharacterSelect::~CharacterSelect()
{
	Release();
}

void CharacterSelect::Init()
{
	// 常に 4 タイプを用意
	typeNames_.clear();
	for (int i = 0; i < TYPE_COUNT; ++i) {
		char buf[32];
		std::snprintf(buf, sizeof(buf), "Type %d", i + 1);
		typeNames_.push_back(std::string(buf));
	}

	selectedIndex_.assign(playerCount_, 0);
	confirmed_.assign(playerCount_, false);

	for (int i = 0; i < 256; ++i) prevKeyState_[i] = 0;
}


// 指定タイプが別プレイヤーに確定されているか（excludePlayer は自分自身を除外する際に使用）
static bool isTypeTaken(const std::vector<int>& selectedIndex, const std::vector<bool>& confirmed, int typeIdx, int excludePlayer = -1)
{
	for (size_t p = 0; p < confirmed.size(); ++p)
	{
		if ((int)p == excludePlayer) continue;
		if (!confirmed[p]) continue;
		if (selectedIndex[p] == typeIdx) return true;
	}
	return false;
}

// 指定タイプの所有者（確定したプレイヤーID）、いなければ -1
static int getTypeOwner(const std::vector<int>& selectedIndex, const std::vector<bool>& confirmed, int typeIdx)
{
	for (size_t p = 0; p < confirmed.size(); ++p)
	{
		if (confirmed[p] && selectedIndex[p] == typeIdx) return static_cast<int>(p);
	}
	return -1;
}

void CharacterSelect::HandleInput()
{
	unsigned char keyState[256];
	GetHitKeyStateAll((char*)keyState);

	auto isKeyDown = [&](int key) {
		return keyState[key] != 0 && prevKeyState_[key] == 0;
		};

	// 左 / 右 でキャラを変更（確定済みタイプはスキップ）
	if (isKeyDown(KEY_INPUT_LEFT) || isKeyDown(KEY_INPUT_RIGHT))
	{
		bool left = isKeyDown(KEY_INPUT_LEFT);
		int curr = selectedIndex_[currentPlayer_];
		int tries = 0;
		while (++tries <= TYPE_COUNT)
		{
			// move
			if (left) curr = (curr - 1 + TYPE_COUNT) % TYPE_COUNT;
			else curr = (curr + 1) % TYPE_COUNT;

			// 自分自身が既に確定しているタイプはそのま許可する（移動は不要だがここは安全対策）
			if (isTypeTaken(selectedIndex_, confirmed_, curr, currentPlayer_)) continue;

			// 選択可能なタイプを見つけたら設定
			selectedIndex_[currentPlayer_] = curr;
			break;
		}
	}

	// Enter で確定（確定済みタイプが他に取られていたら別の空きタイプへ移す）
	if (isKeyDown(KEY_INPUT_RETURN))
	{
		int want = selectedIndex_[currentPlayer_];
		if (isTypeTaken(selectedIndex_, confirmed_, want, currentPlayer_))
		{
			// 既に他が確定していた -> 空きタイプを探して移動する
			bool found = false;
			for (int i = 0; i < TYPE_COUNT; ++i)
			{
				if (!isTypeTaken(selectedIndex_, confirmed_, i, currentPlayer_))
				{
					selectedIndex_[currentPlayer_] = i;
					found = true;
					break;
				}
			}
			if (!found) {
				// 全部取られている（理論上起きない）-> 確定拒否
			}
		}

		// 最終確認（自分以外に取られていなければ確定）
		if (!isTypeTaken(selectedIndex_, confirmed_, selectedIndex_[currentPlayer_], currentPlayer_))
		{
			confirmed_[currentPlayer_] = true;
			// 次の未確定プレイヤーへ移動
			for (int i = 0; i < playerCount_; ++i)
			{
				int next = (currentPlayer_ + 1 + i) % playerCount_;
				if (!confirmed_[next]) { currentPlayer_ = next; break; }
			}
		}
		// もし確定できなければ何もしない（UI上は TAKEN 表示される）
	}

	// Tab で操作中プレイヤーを切り替え
	if (isKeyDown(KEY_INPUT_TAB))
	{
		currentPlayer_ = (currentPlayer_ + 1) % playerCount_;
	}

	// R でリセット（未確定に戻す）
	if (isKeyDown(KEY_INPUT_R))
	{
		std::fill(confirmed_.begin(), confirmed_.end(), false);
		currentPlayer_ = 0;
		// 他プレイヤーに取られて自分の選択が既に使えなくなっている場合、利用可能なものへ補正
		for (int p = 0; p < playerCount_; ++p) {
			if (isTypeTaken(selectedIndex_, confirmed_, selectedIndex_[p], p)) {
				// 空き候補に切り替え
				for (int t = 0; t < TYPE_COUNT; ++t) {
					if (!isTypeTaken(selectedIndex_, confirmed_, t, p)) { selectedIndex_[p] = t; break; }
				}
			}
		}
	}

	// 全プレイヤーが確定したら ApplySelection を呼ぶ
	bool all = std::all_of(confirmed_.begin(), confirmed_.end(), [](bool v) { return v; });
	if (all && !finished_)
	{
		ApplySelection();
		finished_ = true;
	}

	// prev 更新
	memcpy(prevKeyState_, keyState, sizeof(keyState));
}

void CharacterSelect::Update()
{
	if (finished_) return;
	HandleInput();
}

void CharacterSelect::Draw()
{
	ClearDrawScreen();

	const int baseX = 60;
	const int baseY = 80;
	const int lineH = 28;

	DrawFormatString(20, 20, GetColor(255, 255, 0), "CHARACTER SELECT");

	for (int p = 0; p < playerCount_; p++)
	{
		int y = baseY + p * (lineH * 2);

		// プレイヤー見出し
		if (p == currentPlayer_)
			DrawFormatString(baseX, y, GetColor(255, 255, 255), ">> Player %d <<", p + 1);
		else
			DrawFormatString(baseX, y, GetColor(180, 180, 180), "   Player %d   ", p + 1);

		// 選択中のタイプ表示（選択不可なら TAKEN 表示）
		int sel = selectedIndex_[p];
		int owner = getTypeOwner(selectedIndex_, confirmed_, sel);
		if (owner >= 0 && owner != p)
		{
			DrawFormatString(baseX, y + lineH, GetColor(255, 100, 100), "  %s  (TAKEN by P%d)", typeNames_[sel].c_str(), owner + 1);
		}
		else
		{
			DrawFormatString(baseX, y + lineH, GetColor(0, 255, 0), "  %s  %s", typeNames_[sel].c_str(), confirmed_[p] ? "(CONFIRMED)" : "");
		}

		// 簡易プレビュー球（最大4人を見やすく配置）
		float previewX = 200.0f + (float)p * 120.0f - (playerCount_ > 2 ? (playerCount_ - 2) * 60.0f : 0.0f);
		VECTOR pos = { previewX, 200.0f, 0.0f };
		DrawSphere3D(pos, 40.0f, 16, GetColor(100 + p * 30, 100, 255 - p * 40), GetColor(100, 100, 255), TRUE);
	}

	// 全タイプと誰が取っているかも表示（右側）
	for (int t = 0; t < TYPE_COUNT; ++t)
	{
		int y = 80 + t * 24;
		int owner = getTypeOwner(selectedIndex_, confirmed_, t);
		if (owner >= 0)
			DrawFormatString(520, y, GetColor(255, 150, 0), "%s : TAKEN by P%d", typeNames_[t].c_str(), owner + 1);
		else
			DrawFormatString(520, y, GetColor(200, 200, 200), "%s : Available", typeNames_[t].c_str());
	}

	// 操作説明
	int yins = baseY + playerCount_ * (lineH * 2) + 20;
	DrawFormatString(20, yins, GetColor(255, 255, 255), "LEFT / RIGHT : change character (skips taken)");
	DrawFormatString(20, yins + lineH, GetColor(255, 255, 255), "TAB : switch player");
	DrawFormatString(20, yins + lineH * 2, GetColor(255, 255, 255), "ENTER : confirm");
	DrawFormatString(20, yins + lineH * 3, GetColor(255, 255, 255), "R : reset confirms");
	if (finished_)
	{
		DrawFormatString(20, yins + lineH * 5, GetColor(0, 255, 0), "All confirmed. Players created.");
	}
}

void CharacterSelect::ApplySelection()
{
	// PlayerManager にプレイヤーを生成
	PlayerManager::CreateInstance();
	PlayerManager& pm = PlayerManager::GetInstance();

	PlayerParam param; // デフォルトパラメータ、必要に応じて調整／拡張

	for (int p = 0; p < playerCount_; ++p)
	{
		int typeIdx = selectedIndex_[p];
		PlayerType type = PlayerType::Player_1;
		if (typeIdx >= 0 && typeIdx < TYPE_COUNT) {
			type = static_cast<PlayerType>(typeIdx);
		}
		else {
			type = PlayerType::Player_1;
		}
		pm.CreatePlayer(type, p, param);
	}
}

void CharacterSelect::Release()
{
	// 特殊リソースなし
}