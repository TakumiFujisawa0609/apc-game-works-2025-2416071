#pragma once
#include "SceneBase.h"
#include <DxLib.h>

// プレイヤー人数選択シーン
class PlayerNumScene : public SceneBase
{
public:

	// コンストラクタ
	PlayerNumScene(void);
	// デストラクタ
	~PlayerNumScene(void) override;
	void Init(void) override;
	void Update(void) override;
	void Draw(void) override;
	void Release(void) override;

	// 選択中の人数を取得
	int GetSelectNum(void) const { return selectNum_ + 1; }


private:
	// 選択肢
	enum SELECT
	{
		SELECT_1P = 0,
		SELECT_2P,
		SELECT_3P,
		SELECT_4P,
		SELECT_MAX
	};
	// 選択中の番号
	int selectNum_;
};

