#pragma once
#include "Player.h"
#include "Control/InputController.h"

class Player_3 : public Player
{
public:

	static constexpr float MAX_SLIDE_SPEED = 10000.0f; // プレイヤーが滑る最大速度

	// コンストラクタ: 基底クラスPlayerのコンストラクタに引数を渡す
	Player_3(int id, const PlayerParam& param, std::unique_ptr<InputController> controller)
		: Player(id, param, std::move(controller)) {
	}

	~Player_3() override = default;

	// 基本処理
	void Init() override;
	void Release() override;

};

