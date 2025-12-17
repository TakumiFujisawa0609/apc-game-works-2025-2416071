#pragma once
#include "SceneBase.h"
#include <vector>
#include <DxLib.h>

class Result : public SceneBase
{
public:
	// コンストラクタ
	Result(void);

	// デストラクタ
	~Result(void) override;
	void Init(void) override;
	void Update(void) override;
	void Draw(void) override;
	void Release(void) override;

private:
	// 勝者ID（-1=ゲームオーバー/勝者なし）
	int playerWinID_;
	// スコア等（未使用のまま維持）
	int playerScores_[4];

	// 追加: 表示するモデル群
	std::vector<int> modelHandles_;
	std::vector<VECTOR> modelPositions_;
	float modelScale_ = 1.4f;

	// 1人プレイか
	bool singlePlayer_ = false;

	// モデルロード/配置
	void BuildModelsForResult();
};
