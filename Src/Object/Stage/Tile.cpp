#include "Tile.h"
#include <DxLib.h>
#include <chrono>

void Tile::Init(TileType type, int hp, const VECTOR& pos)
{
	type_ = type;
	hp_ = hp;
	pos_ = pos;
	isHole_ = false;
	lastStepTime_ = 0;
}

void Tile::Damage(int amount)
{
	if (isHole_) return;
	if (type_ == TileType::Safe) return;
	hp_ -= amount;

	if (hp_ <= 0)
	{
		isHole_ = true;
	}
}

void Tile::OnStep()
{
	if (isHole_) return;
	if (type_ == TileType::Safe) return;
	int nowMs = GetNowCount();

	if (lastStepTime_ == 0 || (nowMs - lastStepTime_) >= STEP_DAMAGE_INTERVAL)
	{
		lastStepTime_ = nowMs;
		Damage(1);
	}
}

void Tile::Render(int modelHandle) const
{
	if (isHole_) return;
	if (modelHandle < 0) return;
	MV1SetPosition(modelHandle, pos_);
	MV1DrawModel(modelHandle);
}
