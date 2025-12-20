//#include "BulletManager.h"
//
//BulletManager& BulletManager::GetInstance()
//{
//	static BulletManager instance;
//	return instance;
//}
//
//void BulletManager::AddBullet(const VECTOR& pos, const VECTOR& dir, float speed, int ownerId)
//{
//	bullets_.push_back(std::make_shared<Bullet>(pos, dir, speed,ownerId));
//}
//
//void BulletManager::Update()
//{
//
//	// 各種オブジェクトに更新処理を任せる
//	for (auto& b : bullets_)
//	{
//		if(b) b->Update();
//	}
//
//	// 消滅した弾を削除
//	bullets_.erase(
//		std::remove_if(bullets_.begin(), bullets_.end(),
//			[](const std::shared_ptr<Bullet>& b) { return !b || !b->IsAlive(); }),
//		bullets_.end());
//}
//
//void BulletManager::Draw()
//{
//	// 各種オブジェクトに描画処理を任せる
//	for (const auto& b : bullets_)
//	{
//		if (b) b->Draw();
//	}
//}
//
//void BulletManager::Release()
//{
//	bullets_.clear();
//}
