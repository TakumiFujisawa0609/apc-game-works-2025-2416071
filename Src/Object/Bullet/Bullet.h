#pragma once
#include <DxLib.h>


class Bullet
{
public:



	// コンストラクタ
	Bullet(const VECTOR& pos, const VECTOR& dir, float speed);

	// デストラクタ
	~Bullet();

	// 更新処理
	void Update();

	// 描画処理
	void Draw() const;

	void Release();

	bool IsAlive() const { return isAlive_; }
	const VECTOR& GetPos() const { return pos_; }

private:

	VECTOR pos_;		// 弾の位置
	VECTOR dir_;		// 弾の進行方向
	float speed_;		// 弾の速度
	//int modelId_;		// 弾のモデルID
	bool isAlive_;		// 弾の生存状態
};

