#pragma once
#include <DxLib.h>


class Bullet
{
public:

	// 定数
	static constexpr float BULLET_LIMIT = 2000.0f;

	// コンストラクタ
	Bullet(const VECTOR& pos, const VECTOR& dir, float speed, int ownerId = -1);

	// デストラクタ
	~Bullet();

	// 更新処理
	void Update();

	// 描画処理
	void Draw() const;

	void Release();

	bool IsAlive() const { return isAlive_; }
	const VECTOR& GetPos() const { return pos_; }

	// 当たり半径
	float GetCollRad() const { return 30.0f; }

	// 弾を殺す処理
	void Kill() { isAlive_ = false; }

	// 発射者ID取得
	int GetOwnerId() const { return ownerId_; }

private:

	VECTOR pos_;		// 弾の位置
	VECTOR dir_;		// 弾の進行方向
	float speed_;		// 弾の速度
	//int modelId_;		// 弾のモデルID
	bool isAlive_;		// 弾の生存状態
	int ownerId_;		// 発射者のプレイヤーID（-1=なし）
};
