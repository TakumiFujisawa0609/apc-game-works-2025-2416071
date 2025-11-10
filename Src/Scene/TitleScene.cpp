#include <cmath>
#include <DxLib.h>
#include "../Manager/InputManager.h"
#include "../Manager/SceneManager.h"
#include "../Object/Grid.h"
#include "../Object/UIInput.h"
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

}

void TitleScene::Update(void)
{
	// グリッド更新
	grid_->Update();

	// シーン遷移
	InputManager& ins = InputManager::GetInstance();
	SceneManager& scene = SceneManager::GetInstance();

	if(ins.IsTrgDown(KEY_INPUT_0))
	{
		scene.ChangeScene(SceneManager::SCENE_ID::GAME);
	}

	auto ti = UIInput::GetTitleInput();

	// 遷移判定 (PAD or Keyboard)
	if (ti.goPlayerSelect || ti.keyboardGoPlayerSelect) {
		scene.ChangeScene(SceneManager::SCENE_ID::PLAYERNUMBERSELECT);
		return;
	}
	if (ti.goHowToPlay || ti.keyboardGoHowToPlay) {
		scene.ChangeScene(SceneManager::SCENE_ID::MANUAL);
		return;
	}

}

void TitleScene::Draw(void)
{
	// グリッド描画
	grid_->Draw();

	// ガイド表示
	int y = 50;
	DrawFormatString(100, y, GetColor(255, 255, 255), "タイトル");
	y += 40;
	DrawFormatString(100, y, GetColor(200, 200, 200), "Enterキー(Bボタン) : 人数選択");
	y += 20;
	DrawFormatString(100, y, GetColor(200, 200, 200), "Spaceキー(Aボタン) : 操作説明");
	y += 20;
	DrawFormatString2(100, y, GetColor(255, 255, 255), -1, "0 : デバッグモード");

	// 背景色の水色は絶対に見せてはならないので、背景画像は描画必須


}

void TitleScene::Release(void)
{
	// グリッド解放
	grid_->Release();
	delete grid_;
}

