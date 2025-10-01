#include "Player_3.h"
#include <DxLib.h>

void Player_3::Init(void)
{
	// 初期化処理
	pos_ = VGet(10.0f, 1.0f, 0.0f);
	radius_ = 1.0f;
	height_ = 2.0f;
	speed_ = 0.1f;
}

void Player_3::Update(void)
{
	// 移動処理（簡易）
	if (CheckHitKey(KEY_INPUT_I))    pos_.z += speed_;
	if (CheckHitKey(KEY_INPUT_K))    pos_.z -= speed_;
	if (CheckHitKey(KEY_INPUT_J))    pos_.x -= speed_;
	if (CheckHitKey(KEY_INPUT_L))    pos_.x += speed_;
}

void Player_3::Attack(void)
{
	// 攻撃処理をここに実装
	// 例: 弾を発射する、近接攻撃を行うなど
}

void Player_3::Draw(void)
{
	// デバッグ
	DrawFormatString(500, 0, GetColor(255, 255, 255), "Player_3");
}

void Player_3::Release(void)
{
	// リソース解放処理があればここに実装
}
