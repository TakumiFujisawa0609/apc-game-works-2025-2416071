#include <cmath>
#include <DxLib.h>
#include "../Manager/InputManager.h"
#include "../Manager/SceneManager.h"
#include "../Object/Grid.h"
#include "../Manager/Camera.h"
#include "TitleScene.h"

TitleScene::TitleScene(void) : SceneBase()
{
	grid_ = nullptr;
}

TitleScene::~TitleScene(void)
{
}

void TitleScene::Init(void)
{
	// カメラモード変更
	Camera* camera = SceneManager::GetInstance().GetCamera();
	camera->ChangeMode(Camera::MODE::FREE);

	// グリッド初期化
	grid_ = new Grid();
	grid_->Init();

	image_ = LoadGraph("Data/Image/Title_1.png");

}

void TitleScene::Update(void)
{
	// グリッド更新
	grid_->Update();

	// シーン遷移
	InputManager& ins = InputManager::GetInstance();
	SceneManager& scene = SceneManager::GetInstance();

	if (ins.IsTrgDown(KEY_INPUT_B))
	{
		// 実装後にデバッグをすること。
		// 例外スロー起きます
		scene.ChangeScene(SceneManager::SCENE_ID::MANUAL);
	}
	else if  (ins.IsTrgDown(KEY_INPUT_A))
	{
		scene.ChangeScene(SceneManager::SCENE_ID::PLAYERNUMBERSELECT);
	}
	else if(ins.IsTrgDown(KEY_INPUT_0))
	{
		scene.ChangeScene(SceneManager::SCENE_ID::GAME);
	}

}

void TitleScene::Draw(void)
{
	// グリッド描画
	grid_->Draw();

	// タイトル表示
	DrawFormatString2(100, 100, GetColor(255, 255, 255), -1, "Title Scene");
	DrawFormatString2(100, 140, GetColor(255, 255, 255), -1, "A to PLAYERNUMBERSELECT");
	DrawFormatString2(100, 180, GetColor(255, 255, 255), -1, "B to Manual");

	// 背景色の水色は絶対に見せてはならないので、背景画像は描画必須

	DrawGraph(0, 0, image_, true);
}

void TitleScene::Release(void)
{
	// グリッド解放
	grid_->Release();
	delete grid_;

	DeleteGraph(image_);
}

