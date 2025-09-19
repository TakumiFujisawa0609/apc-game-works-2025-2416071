#pragma once
#include <DxLib.h>
class Camera;

class Game
{
public:
	
	//コンストラクタ
	Game();

	//デストラクタ
	~Game();

	//初期化
	bool Init();

	//メインループ
	void Run();

	//終了処理
	void End();

private:

	//モデルハンドル
	int modelHandleId_;	

	//カメラの関数渡し
	Camera* camera_;

	
};

