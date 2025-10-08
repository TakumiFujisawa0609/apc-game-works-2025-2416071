#pragma once
#include <DxLib.h>
#include "../Stage/Stage.h"

class Player
{
public:
	// コンストラクタ・デストラクタ
	Player(int id, float weight = 1.0f);
	virtual ~Player();

	// 基本処理
	virtual void Init();
	virtual void Update();
	virtual void Draw();

	// 解放は派生クラスで実装
	virtual void Release() {};

	// ID取得
	int GetID() const { return id_; }

	// 重さ取得
	float GetWeight() const { return weight_; }

	// 座標取得
	const VECTOR& GetPos() const { return pos_; }

protected:

	// 派生クラスで動きを実装
	// 今後純粋仮想関数化
	virtual void Move();

	// 座標補正
	void ApplyStageGround(const Stage& stage);

	int id_;         // プレイヤーID
	float weight_;  // プレイヤーの重さ（ステージの傾きに影響）
	VECTOR pos_;    // プレイヤーの座標
	VECTOR moveVec_; // 移動ベクトル
	float speed_;   // 移動速度
	int modelId_; // モデルID
};