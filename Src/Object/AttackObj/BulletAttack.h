#pragma once
#include "Common/AttackObj.h"

class BulletAttack : public AttackObj
{
public:

	// コンストラクタ
	BulletAttack(int ownerId, const VECTOR& startPos, const VECTOR& direction, float speed);

	// AttackObjの仮想関数の実装
	virtual void Update() override;
	virtual void Draw() override;
	virtual void OnHitPlayer(Player& player) override;
	virtual void Release() override;

	// 弾の当たり判定用半径を取得
	float GetRadius() const { return radius_; }

private:

	VECTOR dir_;			// 進行方向
	int lifeTimer_ = 120;	// 弾の寿命（フレーム数）
	float radius_ = 20.0f;	// 弾の当たり判定用半径
};

