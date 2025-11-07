#include "BulletManager.h"

BulletManager& BulletManager::GetInstance()
{
	static BulletManager instance;
	return instance;
}

void BulletManager::AddBullet(const VECTOR& pos, const VECTOR& dir, float speed)
{
	bullets_.push_back(std::make_shared<Bullet>(pos, dir, speed));
}

void BulletManager::Update()
{

	for (auto& b : bullets_)
	{
		b->Update();
	}

	// Á–Å‚µ‚½’e‚ğíœ
	bullets_.erase(
		std::remove_if(bullets_.begin(), bullets_.end(),
			[](const std::shared_ptr<Bullet>& b) { return !b->IsAlive(); }),
		bullets_.end());
}

void BulletManager::Draw()
{
	for (const auto& b : bullets_)
	{
		b->Draw();
	}
}

void BulletManager::Release()
{
	bullets_.clear();
}
