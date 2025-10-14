#pragma once
#include <DxLib.h>
#include <memory>
#include "../Stage/Stage.h"

class InputController;

class Player
{
private:
	// 死亡順序を管理するための静的カウンタ
	static int nextDeathOrder_;

protected:
	// 死亡順序 (0: 未死亡, 1: 1番目に死亡, ...)
	int deathOrder_ = 0;

	// 死亡時の処理をまとめる
	void Die();

public:

	static constexpr float GRAVITY = 9.81f;
	static constexpr float SLIDE_FACTOR = 0.5f;
	static constexpr float PLAYER_FRICTION = 0.85f;
	static constexpr float MAX_SPEED = 15.0f;
	static constexpr float INPUT_ACCEL_FACTOR = 1.0f / 10.0f;

	// 外部から静的カウンタをリセットするためのメソッド
	static void ResetDeathCounter() { nextDeathOrder_ = 1; }

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

	// 生存状態取得
	bool IsAlive() const { return isAlive_; }

	// 死亡順序取得
	int GetDeathOrder() const { return deathOrder_; }

	// 当たり判定用の半径取得
	float GetCollisionRadius() const { return collisionRadius_; }



protected:

	// プレイヤーの動き
	virtual void Move();
	VECTOR pos_;
	// 座標補正
	void ApplyStageGround(const Stage& stage);

	int id_;
	float weight_;
	
	VECTOR moveVec_;
	float speed_;
	int modelId_;

	// プレイヤーの生存状態
	bool isAlive_ = true;

	float collisionRadius_ = 70.0f;	// 当たり判定用の半径


	std::unique_ptr<InputController> controller_;
};