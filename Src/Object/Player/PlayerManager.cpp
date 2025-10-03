
#include "PlayerManager.h"
#include "Player_1.h"
#include "Player_2.h"
#include "Player_3.h"
#include "Player_4.h"
#include "Player.h"
#include <DxLib.h>

PlayerManager* PlayerManager::instance_ = nullptr;


PlayerManager::PlayerManager()
{
}

PlayerManager::~PlayerManager()
{
}


void PlayerManager::CreateInstance(void)
{
    if (instance_ == nullptr)
    {
        instance_ = new PlayerManager();
    }
    instance_->Init();
}

PlayerManager& PlayerManager::GetInstance(void)
{
    if (instance_ == nullptr)
    {
        PlayerManager::CreateInstance();
    }
    return *instance_;
}

void PlayerManager::Init(void)
{
	players_.clear();
}


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

std::vector<Player*> PlayerManager::GetPlayerRawPlayers() const
{
	std::vector<Player*> rawPlayers; 
	for (const auto& player : players_)
	{
		rawPlayers.push_back(player.get());
	}
	return rawPlayers;
}



void PlayerManager::CheckCollWithStage(Stage& stage)
{
    for (auto& player : players_)
    {
        VECTOR pos = player->GetPos();
        float r = player->GetRadius();
        float h = player->GetHeight();

        // XZ制限（円柱）
        const CylinderCollider& col = stage.GetCollider();
        float dx = pos.x - col.center.x;
        float dz = pos.z - col.center.z;
        float dist = sqrtf(dx * dx + dz * dz);
        if (dist + r > col.radius)
        {
            float push = col.radius - r;
            float angle = atan2f(dz, dx);
            pos.x = col.center.x + push * cosf(angle);
            pos.z = col.center.z + push * sinf(angle);
        }

        // レイを飛ばして接地
        VECTOR from = VGet(pos.x, pos.y + 1000.0f, pos.z);
        VECTOR to = VGet(pos.x, pos.y - 1000.0f, pos.z);
        MV1_COLL_RESULT_POLY result = MV1CollCheck_Line(stage.GetModelID(), -1, from, to);

        if (result.HitFlag)
        {
            pos.y = result.HitPosition.y + h / 2.0f; // 足元を接地
        }

        player->SetPos(pos);
    }
}
