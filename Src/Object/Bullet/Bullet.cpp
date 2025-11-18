#include "Bullet.h"

Bullet::Bullet(const VECTOR& pos, const VECTOR& dir, float speed, int ownerId)
	:pos_(pos), dir_(dir), speed_(speed), isAlive_(true), ownerId_(ownerId)
{
}

Bullet::~Bullet()
{
}

void Bullet::Update()
{
	

	pos_ = VAdd(pos_, VScale(dir_, speed_));

	// âÊñ äOÇ…èoÇΩÇÁè¡ñ≈
	if (pos_.x < -BULLET_LIMIT || pos_.x > BULLET_LIMIT ||
		pos_.y < -BULLET_LIMIT || pos_.y > BULLET_LIMIT ||
		pos_.z < -BULLET_LIMIT || pos_.z > BULLET_LIMIT)
	{
		isAlive_ = false;
	}
}

void Bullet::Draw() const
{
	DrawSphere3D(pos_, 30.0f, 16, GetColor(255, 0, 0), GetColor(255, 0, 0), TRUE);
}

void Bullet::Release()
{
	// ì¡Ç…Ç»Çµ
}
