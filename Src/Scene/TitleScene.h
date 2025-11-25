#pragma once
#include "SceneBase.h"
class Grid;

class TitleScene : public SceneBase
{

public:

	// コンストラクタ
	TitleScene(void);

	// デストラクタ
	~TitleScene(void) override;

	void Init(void) override;
	void Update(void) override;
	void Draw(void) override;
	void Release(void) override;

private:

	// タイトル画像用変数
	int titleImg_;

	// Bボタン画像
	int bButtonImg_;
};


