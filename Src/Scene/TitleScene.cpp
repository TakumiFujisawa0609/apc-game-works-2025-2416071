#include <cmath>
#include <DxLib.h>
#include "../Manager/InputManager.h"
#include "../Manager/SceneManager.h"
#include "../Object/Grid.h"
#include "../Object/UIInput.h"
#include "../Object/Player/Common/PlayerManager.h"
#include "../Manager/Camera.h"
#include "../Application.h"
#include "TitleScene.h"


TitleScene::TitleScene(void) : SceneBase()
{
	//grid_ = nullptr;
}

TitleScene::~TitleScene(void)
{
}

void TitleScene::Init(void)
{
	// カメラモード変更
	Camera* camera = SceneManager::GetInstance().GetCamera();
	camera->ChangeMode(Camera::MODE::FREE);

	// 再プレイ扱いになるので、PlayerManagerを初期化
	PlayerManager::GetInstance().Init();

	// タイトル画像(2D画像)の読み込み
	titleImg_ = LoadGraph("Data/Image/Title.png");

	// Bボタン画像の読み込み
	bButtonImg_ = LoadGraph("Data/Image/Bpush.png");



}

void TitleScene::Update(void)
{
	// グリッド更新
	//grid_->Update();

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
	/*if (ti.goHowToPlay || ti.keyboardGoHowToPlay) {
		scene.ChangeScene(SceneManager::SCENE_ID::MANUAL);
		return;
	}*/

}

void TitleScene::Draw(void)
{

	// タイトル描画
	DrawRotaGraph(Application::SCREEN_SIZE_X / 2, Application::SCREEN_SIZE_Y / 2, 0.75, 0.0, titleImg_, TRUE);

	// Bボタン描画
	DrawRotaGraph(Application::SCREEN_SIZE_X / 2 + 25, 550, 0.15, 0.0, bButtonImg_, TRUE);


}

void TitleScene::Release(void)
{
	// 画像開放
	DeleteGraph(titleImg_);
	DeleteGraph(bButtonImg_);

}

