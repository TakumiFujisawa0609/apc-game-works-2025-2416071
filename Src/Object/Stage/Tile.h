#pragma once
#include <DxLib.h>

enum class TileType
{
	Normal,
	Breakable,
	Safe,
};

class Tile
{
public:

	Tile() = default;

	// 基本処理
	void Init(TileType type, int hp, const VECTOR& pos);
	void Damage(int amount);
	void OnStep();		// 踏んでるときの処理
	void Render(int modelHandle) const;
	bool IsHole() const { return isHole_; }
	const VECTOR& GetPos() const { return pos_; }
	TileType GetType() const { return type_; }

private:

	TileType type_ = TileType::Normal;
	int hp_ = 0;
	bool isHole_ = false;
	VECTOR pos_ = VGet(0.0f, 0.0f, 0.0f);

	// 踏んだのダメージ間隔制御
	int lastStepTime_ = 0;
	static constexpr int STEP_DAMAGE_INTERVAL = 1000;
};

