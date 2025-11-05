#include "BulletAttack.h"
#include "../Player/Common/Player.h"
#include "../../Utility/AsoUtility.h"

// コンストラクタ
BulletAttack::BulletAttack(int ownerId, const VECTOR& startPos, const VECTOR& direction, float speed)
	: AttackObj(ownerId, startPos, speed), dir_(direction)
{
	// モデルの読み込み
	//modelId_ = MV1LoadModel("Data/Model/Bullet.mv1");
}

void BulletAttack::Update()
{
    if (!isAlive_) return;

    // 移動処理
    pos_ = VAdd(pos_, VScale(dir_, speed_));

    // 寿命チェック
    lifeTimer_--;
    if (lifeTimer_ <= 0) {
        isAlive_ = false;
    }
}

void BulletAttack::Draw()
{
    if (!isAlive_) return;

    // デバッグ用の球体表示
    DrawSphere3D(pos_, radius_, 16, GetColor(255, 255, 0), GetColor(255, 255, 0), TRUE);
}

void BulletAttack::OnHitPlayer(Player& target)
{
    // 衝突時の処理は後回し
    isAlive_ = false;
}

void BulletAttack::Release()
{
    if (modelId_ != -1)
    {
        MV1DeleteModel(modelId_);
        modelId_ = -1;
    }
}

