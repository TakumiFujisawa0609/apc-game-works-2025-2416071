#pragma once
#include<iostream>
#include "Player.h"
#include <DxLib.h>
#include "../../Object/Stage/Stage.h"

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
	void Release(void);
	std::string GetCharacterName(void) const override { return "Player_1"; }

protected:

	void UpdateMove(void);


private:

	float velY_ = 0.0f; // Y軸の速度
	const float gravity_ = -0.01f; // 重力加速度
	const float groundLevel_ = 750.0f; // 地面の高さ
	bool isOnGround_ = false; // 地面に接地しているか

};
