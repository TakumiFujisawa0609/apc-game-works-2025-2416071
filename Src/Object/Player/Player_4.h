#pragma once
#include "Player.h"
class Player_4 :
    public Player
{
public:
	// コンストラクタ
	Player_4(int id, float weight = 1.0f)
		: Player(id, weight) {
	}  // 基底クラスのコンストラクタに渡す

	// 基底クラスの仮想関数をオーバーライド
	void Init(void) override;
	void Update(void) override;
	void Draw(void);
};

