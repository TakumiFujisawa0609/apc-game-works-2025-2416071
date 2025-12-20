//#pragma once
//#include "Bullet.h"
//#include <DxLib.h>
//#include <vector>
//#include <memory>
//
//
//class BulletManager
//{
//public:
//
//	// シングルトンインスタンス取得
//	static BulletManager& GetInstance();
//
//	// 弾の追加
//	void AddBullet(const VECTOR& pos, const VECTOR& dir, float speed, int ownerId = -1);
//
//	// 更新処理
//	void Update();
//
//	// 描画処理
//	void Draw();
//
//	// 解放はSrc/Object/Bullet/BulletManager.cppで実装
//	void Release();
//
//	const std::vector<std::shared_ptr<Bullet>>& GetBullets() const { return bullets_; }
//
//private:
//
//	std::vector<std::shared_ptr<Bullet>> bullets_;
//	BulletManager() = default;
//
//};
//
