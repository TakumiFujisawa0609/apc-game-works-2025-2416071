#pragma once
#include "../Common/Player.h"
#include "../Control/InputController.h"

class Player_3 : public Player
{
public:

	static constexpr float MAX_SLIDE_SPEED = 10000.0f; // プレイヤー共通の最大速度(未使用なら削除可)

	// コンストラクタ: 親クラスPlayerのコンストラクタに委譲
	Player_3(int id, const PlayerParam& param, std::unique_ptr<InputController> controller)
		: Player(id, param, std::move(controller)) {
	}

	// 基本処理
	void Init() override;
	void Update() override;
	void Release() override;
};
