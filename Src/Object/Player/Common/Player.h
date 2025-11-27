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
	float friction = 0.85f;	// 摩擦係数
	float maxSpeed = 15.0f;	// 最大速度
};

class Player
{
public:

	// 定数定義
	static constexpr float MODEL_CENTER_TO_FEET = 20.0f;						// モデルの中心から足元までの距離
	static constexpr float GRAVITY_ACCEL = 0.98f;								// 重力加速度
	static constexpr float PUSHBACK_THRESHOLD_Y = 0.5f;							// Y軸方向の押し出し判定しきい値 
	static constexpr float GRAVITY = 9.81f;										// 重力定数
	static constexpr float SLIDE_FACTOR = 0.5f;									// スライド係数
	static constexpr float PLAYER_FRICTION = 0.85f;								// プレイヤーの摩擦係数
	static constexpr float MAX_SPEED = 15.0f;									// プレイヤーの最大速度
	static constexpr float INPUT_ACCEL_FACTOR = 1.0f / 10.0f;					// 入力加速度係数
	static constexpr VECTOR INIT_VEC_NOR = { 0.0f,0.0f,1.0f };					// 初期入力ベクトル正規化値
	static constexpr float STAGE_LOWER_LIMIT_Y = -1000.0f;	
	static constexpr float RESET_POS = 0.0f;// ステージの落下限界Y座標


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
	virtual void Release() = 0;

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

	// 落下状態取得
	bool IsFalling() const { return isFalling_; }

	// 死亡順序取得
	int GetDeathOrder() const { return deathOrder_; }

	// 当たり判定用の半径取得
	float GetCollisionRadius() const { return collisionRadius_; }

	// 当たった時の処理(ノックバック)
	void ApplyHit(const VECTOR& knockBack);

protected:

	// 死亡時の処理をまとめる
	void Die();

	// プレイヤーの動き
	virtual void Move();

	// 共通の攻撃処理
	void Shot();


	static int nextDeathOrder_;		// 死亡順序を管理するための静的カウンタ

	int deathOrder_ = 0;			// 死亡順序 (0: 未死亡, 1: 1番目に死亡, ...)
	int id_;						// プレイヤー固有ID	
	int modelId_ = -1;				// モデルID

	float collisionRadius_ = 70.0f;	// 当たり判定用の半径
	float attackCooldown_ = 0.0f;	// 攻撃クールダウンタイム
	bool isAlive_ = true;			// 生存フラグ
	bool isFalling_ = false;		// 落下中フラグ

	std::unique_ptr<InputController> controller_;		// 入力コントローラ


	VECTOR inputVecNor_;								// ワールド座標系での入力ベクトル
	// プレイヤーの向き
	VECTOR angle_ = { 0.0f, 0.0f, 0.0f };				
	VECTOR moveVec_;
	VECTOR pos_;

	PlayerParam param_;									//	プレイヤーパラメータ
};