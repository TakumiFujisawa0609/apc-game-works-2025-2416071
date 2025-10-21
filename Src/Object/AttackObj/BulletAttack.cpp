#include "BulletAttack.h"
#include "../../Utility/AsoUtility.h"

BulletAttack::BulletAttack(int ownerId, const VECTOR& startPos, const VECTOR& direction, float speed)
	:AttackObj(ownerId, startPos, direction, speed)
{
	// 個別の初期化
	this->radius_ = 30.0f;
	this->damage_ = 20;
}

void BulletAttack::Update()
{
	// 有効状態でなければ更新しない
	if (!isActive_) return;

	// 位置更新
	pos_ = VAdd(pos_, moveVec_);

	// 60FTで約5秒後に無効化
	if (AsoUtility::MagnitudeF(VSub(pos_, VScale(moveVec_, -60.0f * 5.0f))) > 60.0f * 5.0f)
	{
		isActive_ = false;
	}

}

void BulletAttack::Draw()
{
	// 有効状態でなければ描画しない
	if (!isActive_) return;

	// デバッグ表示: 球体で表示
	DrawSphere3D(pos_, radius_, 16, GetColor(255, 255, 0), GetColor(255, 255, 0), TRUE);
}

void BulletAttack::OnCollision(Player& player)
{
	// 衝突時の処理: ダメージを与えて無効化
	// プレイヤー側でダメージ処理を実装している前提
	// player.TakeDamage(damage_, knockback_, moveVec_);
	// 攻撃オブジェクトを無効化
	isActive_ = false;
}

void BulletAttack::Release()
{
	// モデル描画時から解放処理

}
