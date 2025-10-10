#pragma once
#include <DxLib.h>
#include "../Stage/Stage.h"

class Player
{
public:

	static constexpr float GRAVITY = 9.81f;                            // 重力加速度
	static constexpr float SLIDE_FACTOR = 0.5f;                        // 滑り落ちる力の強さ
	static constexpr float PLAYER_FRICTION = 0.85f;                    // 摩擦による減衰
	static constexpr float MAX_SPEED = 15.0f;                          // プレイヤーの最大速度
	static constexpr float INPUT_ACCEL_FACTOR = 1.0f / 10.0f;          // 操作による加速の強さ調整



	// コンストラクタ・デストラクタ
	Player(int id, float weight,int inputId);
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

	int inputId_; // 入力ID

	VECTOR worldInputVec; // ワールド入力ベクトル
	
};