#pragma once
#include "Bullet.h"
#include <DxLib.h>
#include <vector>
#include <memory>


class BulletManager
{
public:

	static BulletManager& GetInstance();

	void AddBullet(const VECTOR& pos, const VECTOR& dir, float speed);

	void Update();

	void Draw();

	// ‰ð•ú‚ÍSrc/Object/Bullet/BulletManager.cpp‚ÅŽÀ‘•
	void Release();

	const std::vector<std::shared_ptr<Bullet>>& GetBullets() const { return bullets_; }

private:

	std::vector<std::shared_ptr<Bullet>> bullets_;
	BulletManager() = default;

};

