#pragma once
#include "SceneBase.h"
#include <vector>
#include <string>
#include <DxLib.h>

// キャラ選択シーン (順番制: P1→P2→...)
// 現在手番プレイヤーのみ対応 PAD 操作可。キーボードは常にフォールバック入力として有効。
// キャラクター重複禁止 (確定済みプレイヤーの選択とは被らないように自動スキップ)
class CharacterSelect : public SceneBase
{
public:
	static constexpr int MAX_PLAYERS = 4;
	static constexpr int MAX_CHARACTERS = 4; // 仮キャラ数 (最低でも人数以上を推奨)

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
	void ChangeCharacter(int delta);   // 重複回避しつつキャラ変更
	void DecideCurrent();
	void BackToPrevious();

	// 状態遷移
	void AdvancePlayer();
	void RevertPlayer();
	void CompleteSelection();

	// 重複関連
	bool IsTaken(int characterIdx) const;            // 他プレイヤー(確定済)に取られているか
	int  FindNextAvailable(int start, int dir) const; // start から dir(±1)方向へ空きを探す

	// 描画補助
	void DrawPadWarning();
	void DrawPlayerEntries();
	void DrawGuide();
	void DrawDuplicateNotice(); // 行き場がない場合の注意表示

	// 外部へ選択反映
	void ApplySelection();

	int playerCount_;
	int currentPlayer_;
	bool finished_;

	std::vector<int>  selectedIndex_; // 各プレイヤーの選択キャラ (0～MAX_CHARACTERS-1)
	std::vector<bool> confirmed_;     // 確定フラグ
	std::vector<std::string> characterNames_;

	bool duplicateBlock_; // 現在候補が重複で確定不可 (理論上 ChangeCharacter で回避するので false のはず)
};