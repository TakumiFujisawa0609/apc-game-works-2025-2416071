#include "Player.h"


void Player::UpdateMove(void)
{
	// InputManagerのインスタンス
	InputManager& ins = InputManager::GetInstance();

	// 今後、パッドでの移動処理を追加予定

	// キーボード移動
	if (ins.IsNew(KEY_INPUT_W))    pos_.z += speed_;

	if (ins.IsNew(KEY_INPUT_S))    pos_.z -= speed_;

	if (ins.IsNew(KEY_INPUT_A))    pos_.x -= speed_;

	if (ins.IsNew(KEY_INPUT_D))    pos_.x += speed_;


	// 円形コライダー
	// ステージの円柱コライダーを取得
	const CylinderCollider& col = Stage::GetInstance().GetCollider();

	// まとめて減算ができないので、XとZを別々に処理
	float dx = pos_.x - col.center.x;
	float dz = pos_.z - col.center.z;

	// XZ平面での距離
	float distXZ = sqrtf(dx * dx + dz * dz);

	// 円柱コライダーの内側にいるか
	bool isInside = distXZ + radius_ < col.radius;



	// ジャンプ処理
	if (ins.IsNew(KEY_INPUT_1) && isOnGround_)
	{
		velY_ = jumpPower_;
		isOnGround_ = false;
	}

	// 重力処理
	velY_ += gravity_;
	pos_.y += velY_;

	// 落下判定
	if (isInside)
	{
		// ステージ内なら地面を補正
		float groundY = Stage::GetInstance().RayGroundHeight(pos_);

		// 地面より下に行ったら地面に戻す
		if (pos_.y - height_ / 2.0f < groundY)
		{
			pos_.y = groundY + height_ / 2.0f;
			velY_ = 0.0f;
			isOnGround_ = true;
		}
		else
		{
			isOnGround_ = false;
		}
	}
	else
	{
		// ステージ外なら補正しない
		isOnGround_ = false;
	}
}
