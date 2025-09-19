#pragma once
#include "SceneBase.h"
class PlayerNumSelect;

class CharacterSelect : public SceneBase
{
public:
	// コンストラクタ
	CharacterSelect(void);
	// デストラクタ
	~CharacterSelect(void) override;
	void Init(void) override;
	void Update(void) override;
	void Draw(void) override;
	void Release(void) override;



private:

	// 選択された人数
	int playerNum_;

	PlayerNumSelect* playerNumSelect_;
};
