#pragma once
#include "../../Stage/Stage.h"
#include <DxLib.h>
#include <memory>

class InputController;

struct PlayerParam
{
	float weight = 10.0f;	// 重さ
	float speed = 5.0f;		// 移動速度
	float jumpPower = 5.0f;	// ジャンプ力
	float friction = 0.85f;	// 摩擦
	float maxSpeed = 15.0f;	// 最大移動速度
};

class Player
{
public:

	// 定数
	static constexpr float MODEL_CENTER_TO_FEET = 20.0f;		
	static constexpr float GRAVITY_ACCEL = 0.98f;
	static constexpr float PUSHBACK_THRESHOLD_Y = 0.5f;
	static constexpr float GRAVITY = 9.81f;
	static constexpr float SLIDE_FACTOR = 0.5f;
	static constexpr float PLAYER_FRICTION = 0.85f;
	static constexpr float MAX_SPEED = 15.0f;
	static constexpr float INPUT_ACCEL_FACTOR = 1.0f / 10.0f;
	static constexpr VECTOR INIT_VEC_NOR = { 0.0f,0.0f,1.0f };
	static constexpr float STAGE_LOWER_LIMIT_Y = -1000.0f;
	static constexpr float RESET_POS = 0.0f;

	// 弾数管理（ステップ1で追加した仕様を前提）
	static constexpr int AMMO_MAX = 3;
	static constexpr float AMMO_REGEN_INTERVAL_SEC = 5.0f;

	// 反動（射撃時）
	static constexpr float DEFAULT_RECOIL_FORCE = 12.0f;      // 水平反動の強さ
	static constexpr float DEFAULT_RECOIL_VERTICAL_BONUS = 0.0f; // 空中時の縦反動（今回は0で開始）

	static void ResetDeathCounter() { nextDeathOrder_ = 1; }

	Player(int id, const PlayerParam& param, std::unique_ptr<InputController> controller);
	virtual ~Player();

	// 基本処理
	virtual void Init();
	virtual void Update();
	virtual void Draw();
	virtual void Release() = 0;

	// ゲッター・セッター
	int GetID() const { return id_; }
	float GetWeight() const { return param_.weight; }
	const VECTOR& GetPos() const { return pos_; }
	void SetPos(const VECTOR& newPos) { pos_ = newPos; }
	bool IsAlive() const { return isAlive_; }
	bool IsFalling() const { return isFalling_; }
	int GetDeathOrder() const { return deathOrder_; }
	float GetCollisionRadius() const { return collisionRadius_; }

	// ダメージ処理
	void ApplyHit(const VECTOR& knockBack);

protected:

	// 死亡処理
	void Die();

	// 移動処理
	virtual void Move();

	// 射撃処理
	void Shot();

	static int nextDeathOrder_;

	int deathOrder_ = 0;
	int id_;
	int modelId_ = -1;

	float collisionRadius_ = 70.0f;
	float attackCooldown_ = 0.0f;
	bool isAlive_ = true;
	bool isFalling_ = false;

	std::unique_ptr<InputController> controller_;

	VECTOR inputVecNor_;
	VECTOR angle_ = { 0.0f, 0.0f, 0.0f };
	VECTOR moveVec_;
	VECTOR pos_;

	PlayerParam param_;

	// 弾数管理
	int ammoCount_ = AMMO_MAX;
	float ammoRegenTimerSec_ = 0.0f;

	// 反動
	float recoilForce_ = DEFAULT_RECOIL_FORCE;
	float recoilVerticalBonus_ = DEFAULT_RECOIL_VERTICAL_BONUS;
};