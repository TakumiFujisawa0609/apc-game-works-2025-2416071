#pragma once
#include "SceneBase.h"

class GameOverScene:public SceneBase
{
public:

	//コンストラクタ
	GameOverScene();

	//デストラクタ
	~GameOverScene();

	//初期化
	void Init() override;
	
	//更新
	void Update() override;

	//描画
	void Draw() override;

	//解放
	void End() override;

	//次シーン遷移関数
	SceneBase* NextScene() override;

private:

	//次シーン遷移
	bool nextScene_ = false;
};

