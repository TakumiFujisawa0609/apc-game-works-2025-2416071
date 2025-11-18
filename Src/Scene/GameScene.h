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

	// コンストラクタ
	GameScene();
	// デストラクタ
	~GameScene(void)override;

	// 初期化
	void Init(void)override;
	// 更新
	void Update(void)override;
	// 描画
	void Draw(void)override;
	// 3D描画
	void Draw3D(void);
	// リソースの破棄
	void Release(void)override;

	// プレイ人数の取得
	int GetPlayerNum(void) const { return playerNum_; }

private:

	// ここにメンバ変数を追加していく

	// 選択された人数
	int playerNum_;

	// グリッド線
	//Grid* grid_;

	// プレイヤーマネージャー
	PlayerManager* playerManager_;

	// ステージ
	Stage& stage_ = Stage::GetInstance();

	// デバッグ　プレイヤー
	Player_1* debugPlayer_;

	// 遷移タイマー
	int transitionTimer_;

};
