#pragma once
#include "Player.h"

class Player_1 : public Player
{
public:

	static constexpr float MAX_SLIDE_SPEED = 10000.0f; // プレイヤーが滑る最大速度

	// コンストラクタ
	Player_1(int id, float weight);
	~Player_1() override = default;

	// 基本処理
	void Init() override;
	void Move() override;
	void DebugDraw();

};
