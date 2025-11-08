#pragma once
#include "Bullet.h"
#include <DxLib.h>
#include <vector>
#include <memory>


class BulletManager
{
public:

	// シングルトンインスタンス取得

	static BulletManager& GetInstance();

	void AddBullet(const VECTOR& pos, const VECTOR& dir, float speed, int ownerId = -1);

	void Update();

	void Draw();

	// 解放はSrc/Object/Bullet/BulletManager.cppで実装
	void Release();

	const std::vector<std::shared_ptr<Bullet>>& GetBullets() const { return bullets_; }

private:

	std::vector<std::shared_ptr<Bullet>> bullets_;
	BulletManager() = default;

};

