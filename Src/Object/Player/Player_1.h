#pragma once
#include "Player.h"

class Player_1 : public Player
{
public:

	static constexpr float MAX_SLIDE_SPEED = 10000.0f; // プレイヤーが滑る最大速度

	// コンストラクタ
	Player_1(int id, int inputId, float weight)
		: Player(id, weight, inputId) {
	}  // 基底クラスのコンストラクタに渡す

	~Player_1() override = default;

	// 基本処理
	void Init() override;
	void Release() override;

};
