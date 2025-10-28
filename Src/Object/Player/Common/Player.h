#pragma once
#include "../../Stage/Stage.h"
#include "../../AttackObj/BulletAttack.h"
#include <DxLib.h>
#include <memory>

// 前方宣言
class InputController;

// プレイヤーパラメータ構造体
struct PlayerParam
{
	float weight;    // 重さ
	float speed;     // 速度
	float jumpPower; // ジャンプ力
	float friction;  // 摩擦力
	float maxSpeed;  // 最大速度
};

class Player
{
private:

	// 定数宣言

	// プレイヤーモデルの足元から中心までの距離
	static constexpr float MODEL_CENTER_TO_FEET = 20.0f;

	// 死亡順序を管理する静的メンバ変数
	static int nextDeathOrder_;

	VECTOR inputVecNor_; // 入力方向の正規化ベクトル

	// カプセルの高さ
	static constexpr float CAPSULE_HEIGHT = 217.0f;

protected:

	// 死亡順序		(0は誰も4んでない)
	int deathOrder_ = 0;

	// 死亡時の処理
	void Die();

	// 攻撃処理
	virtual void Attack();

public:

	// 定数
	static constexpr float GRAVITY = 0.98f;			// 重力加速度
	static constexpr float SLIDE_FACTOR = 0.5f;			// 滑り係数
	static constexpr float PLAYER_FRICTION = 0.85f;		// 地面摩擦
	static constexpr float MAX_SPEED = 15.0f;			// 最大速度
	static constexpr float INPUT_ACCEL_FACTOR = 15.0f;

	// 外部から静的カウンタをリセットする
	static void ResetDeathCounter() { nextDeathOrder_ = 1; }

	// コンストラクタ
	// ID,重さ,InpurtController
	Player(int id, const PlayerParam& param, std::unique_ptr<InputController> controller);
	virtual ~Player();

	// 基本処理
	virtual void Init();
	virtual void Update();
	virtual void Draw();
	virtual void Release() ;

	// ID取得
	int GetID() const { return id_; }

	// 重さ取得
	float GetWeight() const { return param_.weight; }

	// 位置取得・設定
	const VECTOR& GetPos() const { return pos_; }
	void SetPos(const VECTOR& pos) { pos_ = pos; }

	// 生存状態
	bool IsAlive() const { return isAlive_; }

	// 死亡順序
	int GetDeathOrder() const { return deathOrder_; }

	// 当たり判定用半径取得
	float GetCollisionRadius() const { return collisionRadius_; }

	// カプセルの高さ取得
	float GetCapsuleHeight() const { return CAPSULE_HEIGHT; }

protected:

	// プレイヤーの移動
	virtual void Move();
	VECTOR pos_;             

	// 位置補正
	void ApplyStageGround(const Stage& stage);

	int id_;				// プレイヤーID
	PlayerParam param_;		// プレイヤーパラメータ
	VECTOR moveVec_;		// 移動ベクトル
	int modelId_;			// モデルID

	// プレイヤーの向き
	VECTOR angle_ = { 0.0f, 0.0f, 0.0f };

	// プレイヤーの生存状態
	bool isAlive_ = true;								// true: 生存中, false: 死亡
	float collisionRadius_ = 30.0f;						// 当たり判定用半径
	std::unique_ptr<InputController> controller_;		// 入力コントローラー

	// 攻撃クールダウン
	int attackCooldown_ = 0;

	// 落下状態
	bool isFalling_ = false;							// true: 落下中, false: 地面上

};