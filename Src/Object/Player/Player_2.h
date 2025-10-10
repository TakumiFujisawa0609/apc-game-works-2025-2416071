#pragma once
#include "Player.h"

class Player_2 : public Player
{
public:

	// コンストラクタ
	Player_2(int id, int inputId, float weight)
		: Player(id, inputId, weight) {
	}  // 基底クラスのコンストラクタに渡す

	// 基底クラスの仮想関数をオーバーライド
	void Init(void) override;
	void Release(void) ;
};

