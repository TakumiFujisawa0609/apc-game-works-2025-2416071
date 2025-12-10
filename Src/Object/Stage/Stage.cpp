#include "../../Utility/AsoUtility.h"
#include "../Player/Common/Player.h"
#include "../Player/Common/PlayerManager.h"
#include "Stage.h"
#include <cmath>
#include "../../Manager/InputManager.h"

// インスタンス管理
Stage* Stage::instance_ = nullptr;

// インスタンス取得
Stage& Stage::GetInstance()
{
	if (instance_ == nullptr)
	{
		CreateInstance();
	}
	return *instance_;
}

void Stage::CreateInstance()
{
	if (instance_ == nullptr)
	{
		instance_ = new Stage();
	}
}

// 初期化
void Stage::Init()
{
	tileModelId_ = MV1LoadModel("Data/Model/Stage/Tile.mv1");
	skyModelId_ = MV1LoadModel("Data/Model/Stage/skydome.mv1");

	for (int z = 0; z < TILE_COUNT; ++z) {
		for (int x = 0; x < TILE_COUNT; ++x) {
			VECTOR pos = TileIdxToWorld(x, z);
			bool isCorner = (x == 0 && z == 0) ||
				(x == 0 && z == TILE_COUNT - 1) ||
				(x == TILE_COUNT - 1 && z == 0) ||
				(x == TILE_COUNT - 1 && z == TILE_COUNT - 1);
			if (isCorner)
				tiles_[x][z].Init(TYPE::SAFE, 9999, pos, tileModelId_);
			else
				tiles_[x][z].Init(TYPE::BRAKABLE, 3, pos, tileModelId_);
		}
	}

	// ステージ基準位置
	pos_ = DEFAULT_POS;
	angle_ = AsoUtility::VECTOR_ZERO;
	scale_ = DEFAULT_SCALE;

	// スカイドーム
	skyPos_ = { 0.0f, 250.0f, 0.0f };
	skyAngle_ = AsoUtility::VECTOR_ZERO;
	skyScale_ = { 50.0f, 50.0f, 50.0f };

	// コライダー
	collider_.center = pos_;
	collider_.radius = COLLIDER_RADIUS;
	collider_.yMin = pos_.y;
	collider_.yMax = pos_.y + COLLIDER_YMAX_OFFSET;

	// 物理回転
	angularVelocity_ = AsoUtility::VECTOR_ZERO;
	momentOfInertia_ = MOMENT_OF_INERTIA;
	dampingFactor_ = DAMPING_FACTOR;
	restitutionFactor_ = 0.0f;

}

// 更新
void Stage::Update(const std::vector<Player*>& players)
{
	// 傾きの物理更新
	angle_.x += angularVelocity_.x;
	angle_.y += angularVelocity_.y;
	angle_.z += angularVelocity_.z;
	angularVelocity_.x *= (1.0f - dampingFactor_);
	angularVelocity_.y *= (1.0f - dampingFactor_);
	angularVelocity_.z *= (1.0f - dampingFactor_);

	// プレイヤーが踏んでいるタイルごとにOnStep(床を壊す)
	CheckPlayerStepOnTiles(players);
}

// 描画
void Stage::Draw()
{
	// スカイドーム描画
	MV1SetPosition(skyModelId_, skyPos_);
	MV1SetRotationXYZ(skyModelId_, skyAngle_);
	MV1SetScale(skyModelId_, skyScale_);
	MV1DrawModel(skyModelId_);

	// タイル描画

	for (int z = 0; z < TILE_COUNT; ++z)
		for (int x = 0; x < TILE_COUNT; ++x)
			tiles_[x][z].Draw(angle_);
}

// 解放
void Stage::Release()
{
	if (skyModelId_ != -1)
	{
		MV1DeleteModel(skyModelId_);
		skyModelId_ = -1;
	}
	if (tileModelId_ != -1)
	{
		MV1DeleteModel(tileModelId_);
		tileModelId_ = -1;
	}
	for (int z = 0; z < TILE_COUNT; ++z)
		for (int x = 0; x < TILE_COUNT; ++x)
			tiles_[x][z].Release();
}

// 傾き物理（プレイヤーの重さ位置で傾く）
void Stage::UpdateTilt(const std::vector<Player*>& players)
{
	InputManager& ins = InputManager::GetInstance();

	if (ins.IsNew(KEY_INPUT_F1))
	{
		return;
	}

	if (players.empty()) return;

	float totalWeight = 0.0f;
	float weightedX = 0.0f;
	float weightedZ = 0.0f;

	for (auto* p : players)
	{
		if (!(p->IsAlive() && !p->IsFalling()))
			continue;

		int tx, tz;
		if (!WorldToTileIndex(p->GetPos(), tx, tz)) continue;
		if (tx < 0 || tx >= TILE_COUNT || tz < 0 || tz >= TILE_COUNT) continue;
		if (tiles_[tx][tz].IsHole()) continue; // 穴にいるプレイヤーは傾き対象外

		float weight = p->GetWeight();
		VECTOR pos = p->GetPos();
		weightedX += (pos.x - this->pos_.x) * weight;
		weightedZ += (pos.z - this->pos_.z) * weight;
		totalWeight += weight;
	}

	if (totalWeight <= 0.0f) return;

	float centerX = weightedX / totalWeight;
	float centerZ = weightedZ / totalWeight;

	float totalMass = totalWeight;
	const float GRAVITY = 9.81f;

	VECTOR torque = AsoUtility::VECTOR_ZERO;
	torque.x = centerZ * totalMass * GRAVITY;
	torque.z = -centerX * totalMass * GRAVITY;

	VECTOR restoringTorque = AsoUtility::VECTOR_ZERO;
	restoringTorque.x = -angle_.x * restitutionFactor_;
	restoringTorque.z = -angle_.z * restitutionFactor_;

	VECTOR totalTorque = VAdd(torque, restoringTorque);

	VECTOR angularAcceleration = VScale(totalTorque, 1.0f / momentOfInertia_);
	float deltaTime = 1.0f / 60.0f;
	angularVelocity_ = VAdd(angularVelocity_, VScale(angularAcceleration, deltaTime));
	angularVelocity_ = VScale(angularVelocity_, dampingFactor_);
	angle_ = VAdd(angle_, VScale(angularVelocity_, deltaTime));

	const float maxTilt = AsoUtility::Deg2RadF(45.0f);
	angle_.x = std::fmax(std::fmin(angle_.x, maxTilt), -maxTilt);
	angle_.z = std::fmax(std::fmin(angle_.z, maxTilt), -maxTilt);

	MV1SetRotationXYZ(tileModelId_, angle_);
	MV1RefreshCollInfo(tileModelId_);

	angle_.y = 0.0f;
}

// プレイヤー座標がステージ内か判定
bool Stage::IsPlayerOnStage(const VECTOR& playerPos) const
{
	VECTOR relationPos = VSub(playerPos, this->pos_);
	relationPos.y = 0.0f;
	float dist = relationPos.x * relationPos.x + relationPos.z * relationPos.z;
	return dist <= (collider_.radius * collider_.radius);
}

// ステージ傾きベクトル取得
VECTOR Stage::GetStageNormal() const
{
	VECTOR up = { 0.0f, 1.0f, 0.0f };
	VECTOR v = VTransform(up, MGetRotX(angle_.x));
	v = VTransform(v, MGetRotZ(angle_.z));
	v = VTransform(v, MGetRotY(angle_.y));
	return v;
}

// ワールド座標→タイル配列インデックス
bool Stage::WorldToTileIndex(const VECTOR& worldPos, int& outTileX, int& outTileZ) const
{
	VECTOR localPos = VSub(worldPos, pos_);
	outTileX = static_cast<int>((localPos.x + (TILE_COUNT * TILE_SIZE) / 2) / TILE_SIZE);
	outTileZ = static_cast<int>((localPos.z + (TILE_COUNT * TILE_SIZE) / 2) / TILE_SIZE);
	bool inRange = (outTileX >= 0 && outTileX < TILE_COUNT && outTileZ >= 0 && outTileZ < TILE_COUNT);
	return inRange;
}

// タイル座標→ワールド座標
VECTOR Stage::TileIdxToWorld(int x, int z) const
{
	float offsetX = (x - TILE_COUNT / 2 + 0.5f) * TILE_SIZE;
	float offsetZ = (z - TILE_COUNT / 2 + 0.5f) * TILE_SIZE;
	return VAdd(VGet(offsetX, 0.0f, offsetZ), pos_);
}

void Stage::CheckPlayerStepOnTiles(const std::vector<Player*>& players)
{
	for (const auto* p : players)
	{
		VECTOR pos = p->GetPos();
		int tx, tz;
		if (WorldToTileIndex(pos, tx, tz))
		{
			if (tx >= 0 && tx < TILE_COUNT && tz >= 0 && tz < TILE_COUNT)
				tiles_[tx][tz].OnStep();
		}
	}
}