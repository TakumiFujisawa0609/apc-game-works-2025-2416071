#include "PlayerManager.h"
#include "Player_1.h"
#include "Player_2.h"
#include "Player_3.h"
#include "Player_4.h"
#include <DxLib.h>

void PlayerManager::CreatePlayer(CharacterType type, int id)
{
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

void PlayerManager::UpdateAllPlayers(void)
{
	for (auto& player : players_)
	{
		player->Update();
	}
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
	//あくまでデバッグ
	for (auto& player : players_)
	{
		player->Attack();
	}
}
