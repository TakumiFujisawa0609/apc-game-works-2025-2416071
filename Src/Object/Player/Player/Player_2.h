#pragma once
#include "../Common/Player.h"
#include "../Control/InputController.h"
#include <DxLib.h>

class Player_2 : public Player
{
public:

	// コンストラクタ: 基底クラスPlayerのコンストラクタに引数を渡す
	Player_2(int id, const PlayerParam& param, std::unique_ptr<InputController> controller)
		: Player(id, param, std::move(controller)) {
	}

	// 基底クラスの仮想関数をオーバーライド
	void Init(void) override;
	void Release(void) ;
};

