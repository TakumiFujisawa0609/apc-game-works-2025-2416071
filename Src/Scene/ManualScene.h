#pragma once
#include "SceneBase.h"


// 操作説明シーン(2D)	タイトル (特定キー)で遷移
class ManualScene : public SceneBase
{
public:

	// コンストラクタ
	ManualScene(void);

	// デストラクタ
	~ManualScene(void) override;

	void Init(void) override;

	void Update(void) override;

	void Draw(void) override;

	void Release(void) override;

private:

	// ここにメンバ変数を追加していく
};