#pragma once
#include<iostream>
#include "Player.h";

class Player_1 : public Player
{
public:

	// コンストラクタ
	Player_1(int id) :Player(id) { }

	// 基底クラスの仮想関数をオーバーライド
	void Init(void) override;
	void Update(void) override;
	void Attack(void) override;
	void Draw(void);
	std::string GetCharacterName(void) const override { return "Player_1"; }

};
