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

    // ゲームプレイ用に PlayerManager 初期化
    PlayerManager::GetInstance().Init();

    // タイトル画像
    titleImg_ = LoadGraph("Data/Image/Title.png");

	// Bボタン画像
    aButtonImg_ = LoadGraph("Data/Image/button/xbox_button_color_a.png");
}

void TitleScene::Update(void)
{
    InputManager& ins = InputManager::GetInstance();
    SceneManager& scene = SceneManager::GetInstance();

    if (ins.IsTrgDown(KEY_INPUT_0))
    {
        scene.ChangeScene(SceneManager::SCENE_ID::GAME);
    }

    auto ti = UIInput::GetTitleInput();

    // 進む：A（Pad）/ Enter（KB）
    if (ti.goPlayerSelect || ti.keyboardGoPlayerSelect) {
        scene.ChangeScene(SceneManager::SCENE_ID::PLAYERNUMBERSELECT);
        return;
    }

    // HowToPlay は未使用（必要なら有効化）
}

void TitleScene::Draw(void)
{
    // タイトル描画
    DrawRotaGraph(Application::SCREEN_SIZE_X / 2, Application::SCREEN_SIZE_Y / 2, 0.75, 0.0, titleImg_, TRUE);

    // 画面下部に操作ガイド
	DrawRotaGraph(Application::SCREEN_SIZE_X / 2, Application::SCREEN_SIZE_Y - 100, 1.0, 0.0, aButtonImg_, TRUE);

    // "ではじめる"を表示
	DrawString(Application::SCREEN_SIZE_X / 2 + 35, Application::SCREEN_SIZE_Y - 105, "ではじめる", GetColor(255, 255, 255));
}

void TitleScene::Release(void)
{
    // 画像解放
    DeleteGraph(titleImg_);
    DeleteGraph(aButtonImg_);
}

