
#include "PlayerManager.h"
#include "Player_1.h"
#include "Player_2.h"
#include "Player_3.h"
#include "Player_4.h"
#include "Player.h"
#include <DxLib.h>

void PlayerManager::CreatePlayer(CharacterType type, int id)
{
    // 同じIDのプレイヤーが既に存在するかチェック
    for (auto& player : players_)
    {
        if (player->GetID() == id)
        {
            return; // 既に存在 → 新しく作らない
        }
    }

    switch (type)
    {
    case CharacterType::Player_1:
        players_.emplace_back(std::make_shared<Player_1>(id));
        break;
    case CharacterType::Player_2:
        players_.emplace_back(std::make_shared<Player_2>(id));
        break;
    case CharacterType::Player_3:
        players_.emplace_back(std::make_shared<Player_3>(id));
        break;
    case CharacterType::Player_4:
        players_.emplace_back(std::make_shared<Player_4>(id));
        break;
    default:
        break;
    }
}

void PlayerManager::InitAllPlayers(void)
{
    for (auto& player : players_)
    {
        player->Init();
    }
}

void PlayerManager::UpdateAllPlayers(Stage& stage)
{
    for (auto& player : players_)
    {
        player->Update();
    }
	
	// ステージとの当たり判定
	CheckCollWithStage(stage);
}

void PlayerManager::DrawAllPlayers(void)
{
    for (auto& player : players_)
    {
        player->Draw();
    }
}

void PlayerManager::AttackAllPlayers(void)
{
    for (auto& player : players_)
    {
        player->Attack();
    }
}

void PlayerManager::ClearPlayers(void)
{
    players_.clear();
}

VECTOR PlayerManager::TransformToStageLocal(const VECTOR& worldPos, const Stage& stage)
{
	VECTOR local = VSub(worldPos, stage.GetPos());

    float angle = -stage.GetAngle().y; // Y軸回転逆
	float x = local.x * cosf(angle) - local.z * sinf(angle);
	float z = local.x * sinf(angle) + local.z * cosf(angle);

	local.x = x;
	local.z = z;

	return local;
}


// ステージローカル座標からワールド座標に変換
VECTOR PlayerManager::TransformToWorldPos(const VECTOR& local, const Stage& stage)
{
	float angle = stage.GetAngle().y; // Y軸回転
	float x = local.x * cosf(angle) - local.z * sinf(angle);
	float z = local.x * sinf(angle) + local.z * cosf(angle);

	VECTOR worldPos = VAdd(VGet(x, local.y, z), stage.GetPos());
	return worldPos;
}


void PlayerManager::CheckCollWithStage(Stage& stage)
{
	const CylinderCollider& col = stage.GetCollider();
	for (auto& player : players_)
	{
		VECTOR pos = player->GetPos();
		float r = player->GetRadius();
		float h = player->GetHeight();
		// XZ平面での距離
		float dx = pos.x - col.center.x;
		float dz = pos.z - col.center.z;
		float dist = sqrtf(dx * dx + dz * dz);
		if (dist + r > col.radius)
		{
			// 円柱の外に出そうなら押し戻す
			float push = col.radius - r;
			float angle = atan2f(dz, dx);
			pos.x = col.center.x + push * cosf(angle);
			pos.z = col.center.z + push * sinf(angle);
		}
		// Y軸制御（床・天井）
		if (pos.y - h / 2 < col.yMin) pos.y = col.yMin + h / 2;
		if (pos.y + h / 2 > col.yMax) pos.y = col.yMax - h / 2;
		player->SetPos(pos);
	}
}