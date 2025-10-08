#pragma once
#include "Player.h"

class Player_2 : public Player
{
public:

	// コンストラクタ
	Player_2(int id, float weight = 1.0f)
		: Player(id, weight) {
	}  // 基底クラスのコンストラクタに渡す

	// 基底クラスの仮想関数をオーバーライド
	void Init(void) override;
	void Update(void) override;
	//void Attack(void) override;
	void Draw(void)override;
	void Release(void) ;
};

