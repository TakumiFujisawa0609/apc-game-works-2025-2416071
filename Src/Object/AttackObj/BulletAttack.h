#pragma once
#include "Common/AttackObj.h"

// 弾丸攻撃オブジェクトクラス
class BulletAttack : public AttackObj
{
public:

	// コンストラクタ
	BulletAttack(int ownerId, const VECTOR& startPos, const VECTOR& direction, float speed);

	// 基底クラスの仮想関数をオーバーライド
	void Update() override;
	void Draw() override;

	// 衝突時の処理
	void OnCollision(Player& player) override;

	// 解放
	void Release();
};