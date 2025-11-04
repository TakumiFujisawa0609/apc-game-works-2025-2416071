#pragma once
#include <DxLib.h>

class Player;

class Attackobj
{
public:

	// コンストラクタ
	Attackobj(int ownerId, const VECTOR& pos, float speed)
		: ownerId_(ownerId), pos_(pos), speed_(speed), isAlive_(true), modelId_(-1) {}

	virtual ~Attackobj() = default;

	// 毎フレーム更新
	// 純粋仮想関数
	virtual void Update() = 0;

	// 描画
	virtual void Draw() = 0;

	// プレイヤーに当たった時の処理
	virtual void OnHitPlayer(Player& player) = 0;

	// 解放
	virtual void Release() {};

	// 座標取得
	const VECTOR& GetPos() const { return pos_; }

	// 生存状態取得
	bool IsAlive() const { return isAlive_; }

	// 所有者ID取得
	int GetOwnerID() const { return ownerId_; }

protected:

	int ownerId_;		// 所有者のプレイヤーID
	VECTOR pos_;		// 座標
	float speed_;		// 速度
	bool isAlive_;		// 生存状態
	int modelId_;		// モデルID


};