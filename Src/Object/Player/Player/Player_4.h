#pragma once
#include "../Common/Player.h"
#include "../Control/InputController.h"

class Player_4 :
	public Player
{
public:
	Player_4(int id, const PlayerParam& param, std::unique_ptr<InputController> controller)
		: Player(id, param, std::move(controller)) {
	}

	// Šî–{ˆ—
	void Init() override;
	void Update() override;
	void Release() override;
};