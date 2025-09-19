#pragma once
#include "StageManager.h"
class Stage_1 : public StageManager
{
public:
	// コンストラクタ
	Stage_1(void);
	// デストラクタ
	virtual ~Stage_1(void);
	// 初期化処理
	virtual void Init(void) override;
	// 更新ステップ
	virtual void Update(void) override;
	// 描画処理
	virtual void Draw(void) override;
	// 解放処理
	virtual void Release(void) override;

	// パラメータ
	void SetParam(int playerNum);

private:


};

