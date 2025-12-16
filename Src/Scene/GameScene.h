#pragma once
#include "SceneBase.h"
#include "../Object/Stage/Stage.h" 

class Grid;
class StageManager;
class PlayerManager;
class Stage;
class Player_1;

class GameScene : public SceneBase
{
public:
	GameScene();
	~GameScene(void)override;

	void Init(void)override;
	void Update(void)override;
	void Draw(void)override;
	void Draw3D(void);
	void Release(void)override;

	int GetPlayerNum(void) const { return playerNum_; }

private:
	int playerNum_;
	PlayerManager* playerManager_;
	Stage& stage_ = Stage::GetInstance();
	Player_1* debugPlayer_;
	int transitionTimer_;

	// ’Ç‰Á: 1lƒvƒŒƒC”»’è‚ğ•Û
	bool singlePlayerMode_ = false;
};