#pragma once
#include <DxLib.h>
#include <memory>
#include "../Stage/Stage.h"

class InputController;

class Player
{
public:

	static constexpr float GRAVITY = 9.81f;
	static constexpr float SLIDE_FACTOR = 0.5f;
	static constexpr float PLAYER_FRICTION = 0.85f;
	static constexpr float MAX_SPEED = 15.0f;
	static constexpr float INPUT_ACCEL_FACTOR = 1.0f / 10.0f;

	// コンストラクタ: 固有ID, 重さ, InputControllerを受け取る
	Player(int id, float weight, std::unique_ptr<InputController> controller);
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

	// プレイヤーの動き
	virtual void Move();

	// 座標補正
	void ApplyStageGround(const Stage& stage);

	int id_;
	float weight_;
	VECTOR pos_;
	VECTOR moveVec_;
	float speed_;
	int modelId_;

	std::unique_ptr<InputController> controller_;
};