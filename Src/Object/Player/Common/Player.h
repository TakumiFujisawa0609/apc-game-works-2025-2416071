#pragma once
#include <DxLib.h>
#include <memory>
#include "../../Stage/Stage.h"
#include "../../AttackObj/BulletAttack.h"


class InputController;

struct PlayerParam
{
	float weight = 10.0f;	// 重さ
	float speed = 5.0f;		// 移動速度
	float jumpPower = 5.0f;	// ジャンプ力
	float friction = 0.85f;	// 摩擦係数
	float maxSpeed = 15.0f;	// 最大速度
};

class Player
{
private:

	// プレイヤーモデルの中心から足元までの距離
	static constexpr float MODEL_CENTER_TO_FEET = 20.0f;
	static constexpr float GRAVITY_ACCEL = 0.98f;
	static constexpr float PUSHBACK_THRESHOLD_Y = 0.5f; // 地面と判断する法線のY成分のしきい値

	// 死亡順序を管理するための静的カウンタ
	static int nextDeathOrder_;

	VECTOR inputVecNor_; // ワールド座標系での入力ベクトル

protected:
	// 死亡順序 (0: 未死亡, 1: 1番目に死亡, ...)
	int deathOrder_ = 0;

	// 死亡時の処理をまとめる
	void Die();

	// 攻撃処理 (派生クラスで実装)
	virtual	void Attack();

public:

	static constexpr float GRAVITY = 9.81f;
	static constexpr float SLIDE_FACTOR = 0.5f;
	static constexpr float PLAYER_FRICTION = 0.85f;
	static constexpr float MAX_SPEED = 15.0f;
	static constexpr float INPUT_ACCEL_FACTOR = 1.0f / 10.0f;

	// 外部から静的カウンタをリセットするためのメソッド
	static void ResetDeathCounter() { nextDeathOrder_ = 1; }

	// コンストラクタ: 固有ID, 重さ, InputControllerを受け取る
	Player(int id, const PlayerParam& param, std::unique_ptr<InputController> controller);
	virtual ~Player();

	// 基本処理
	virtual void Init();
	virtual void Update();
	virtual void Draw();

	// 解放は派生クラスで実装
	virtual void Release();

	// ID取得
	int GetID() const { return id_; }

	// 重さ取得
	float GetWeight() const { return param_.weight; }

	// 座標取得
	const VECTOR& GetPos() const { return pos_; }

	// 座標設定
	void SetPos(const VECTOR& newPos) { pos_ = newPos; }

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
	int id_;
	PlayerParam param_;

	VECTOR moveVec_;
	int modelId_;

	// プレイヤーの向き
	VECTOR angle_ = { 0.0f, 0.0f, 0.0f };


	// プレイヤーの生存状態
	bool isAlive_ = true;

	float collisionRadius_ = 70.0f;	// 当たり判定用の半径


	std::unique_ptr<InputController> controller_;

	// 攻撃クールダウン
	float attackCooldown_ = 0.0f;

	// 落下フラグ
	bool isFalling_ = false;
};