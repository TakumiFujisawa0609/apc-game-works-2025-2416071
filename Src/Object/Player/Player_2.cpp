#include "Player_2.h"
#include <DxLib.h>

void Player_2::Init(void)
{
	// 初期化処理
	pos_ = VGet(5.0f, 1.0f, 0.0f);
	radius_ = 1.0f;
	height_ = 2.0f;
	speed_ = 0.15f;
}

void Player_2::Update(void)
{
	// 移動処理（簡易）
	if (CheckHitKey(KEY_INPUT_W))    pos_.z += speed_;
	if (CheckHitKey(KEY_INPUT_S))    pos_.z -= speed_;
	if (CheckHitKey(KEY_INPUT_A))    pos_.x -= speed_;
	if (CheckHitKey(KEY_INPUT_D))    pos_.x += speed_;
}


void Player_2::Attack(void)
{
	// 攻撃処理をここに実装
	// 例: 弾を発射する、近接攻撃を行うなど
}

void Player_2::Draw(void)
{
	// デバッグ
	DrawFormatString(400, 20, GetColor(255, 255, 255), "Player_2");
}

void Player_2::Release(void)
{
}

