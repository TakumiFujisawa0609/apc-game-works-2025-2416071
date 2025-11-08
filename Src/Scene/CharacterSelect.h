#pragma once
#include "SceneBase.h"
#include <vector>
#include <memory>
#include <DxLib.h>
#include <string>

class CharacterSelect : public SceneBase
{
public:
	// 最大プレイヤー数を明示
	static constexpr int MAX_PLAYERS = 4;

	// デフォルト引数を 0 にして、未指定の場合は SceneManager から取得する
	CharacterSelect(int playerCount = 0);
	virtual ~CharacterSelect() override;

	// SceneBase 実装
	void Init() override;
	void Update() override;
	void Draw() override;
	void Release() override;

	// 選択が完了したか
	bool IsFinished() const { return finished_; }

	// 選択されたプレイヤータイプを反映
	void ApplySelection();

private:
	void HandleInput();

	int playerCount_;
	std::vector<int> selectedIndex_; // Player ごとの選択インデックス (0..numTypes-1)
	std::vector<bool> confirmed_;    // Player ごとの確定フラグ
	int currentPlayer_;              // 操作中のプレイヤー
	bool finished_;

	// キー押下デバウンス
	unsigned char prevKeyState_[256];

	// 表示用ラベル
	std::vector<std::string> typeNames_;
};