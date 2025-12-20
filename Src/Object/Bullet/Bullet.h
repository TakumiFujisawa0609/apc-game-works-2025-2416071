//#pragma once
//#include <DxLib.h>
//
//class Bullet
//{
//public:
//	// 定数
//	static constexpr float BULLET_LIMIT = 2000.0f;
//
//	// コンストラクタ
//	Bullet(const VECTOR& pos, const VECTOR& dir, float speed, int ownerId = -1);
//
//	// デストラクタ
//	~Bullet();
//
//	// 更新処理
//	void Update();
//
//	// 描画
//	void Draw() const;
//
//	void Release();
//
//	bool IsAlive() const { return isAlive_; }
//	const VECTOR& GetPos() const { return pos_; }
//
//	// 衝突半径
//	float GetCollRad() const { return 30.0f; }
//
//	// 弾を消す
//	void Kill() { isAlive_ = false; }
//
//	// オーナーID取得
//	int GetOwnerId() const { return ownerId_; }
//
//	// 追加: 前フレーム位置の取得（CCD用）
//	const VECTOR& GetPrevPos() const { return prevPos_; }
//
//private:
//	VECTOR pos_;        // 弾の位置
//	VECTOR dir_;        // 弾の進行方向（正規化推奨）
//	float speed_;       // 弾の速度
//	bool isAlive_;      // 生存フラグ
//	int ownerId_;       // 発射者のプレイヤーID（-1=なし）
//	VECTOR prevPos_;    // 前フレーム位置（CCD用）
//	bool collidedWithStage_ = false;
//};