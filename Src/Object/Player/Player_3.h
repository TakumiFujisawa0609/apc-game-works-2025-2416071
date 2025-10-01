#pragma once
#include "Player.h"
class Player_3 :
    public Player
{
public:
	// コンストラクタ
	Player_3(int id) :Player(id) {}
	// 基底クラスの仮想関数をオーバーライド
	void Init(void) override;
	void Update(void) override;
	void Attack(void) override;
	void Draw(void)override;
	void Release(void) override;
	std::string GetCharacterName(void) const override { return "Player_3"; }
};

