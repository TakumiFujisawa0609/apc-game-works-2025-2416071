#include "AttackObj.h"

AttackObj::AttackObj(int ownerId, const VECTOR& startPos, const VECTOR& direction, float speed)
{
	ownerId_ = ownerId;
	pos_ = startPos;
	moveVec_ = VScale(direction, speed);
	isActive_ = true;
}

void AttackObj::Init()
{
	// メンバ変数の初期化
	radius_ = 10.0f;
	damage_ = 10;
	knockback_ = 5;

	// 位置と移動ベクトルはコンストラクタで初期化
}
