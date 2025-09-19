#include "Game.h"
#include "Common/Camera.h"

Game::Game(){}
Game::~Game() {}

bool Game::Init()
{
    SetOutApplicationLogValidFlag(FALSE);
    ChangeWindowMode(TRUE);
    if (DxLib_Init() == -1) return false;
    SetDrawScreen(DX_SCREEN_BACK);

    // テストモデル読み込み
    modelHandleId_ = MV1LoadModel("Data/Model/Test.mv1");
    //if (modelHandleId_ == -1) return false;

    // カメラ初期化
    camera_ = new Camera;
    camera_->Init();

   

	//カメラ位置設定
	camera_->SetPosition(VGet(0.0f, 30.0f, -50.0f));


    return true;
}

void Game::Run()
{
    while (ProcessMessage() == 0 && CheckHitKey(KEY_INPUT_ESCAPE) == 0)
    {
        // カメラ操作
        camera_->Update();

        // カメラ設定
        SetCameraPositionAndTarget_UpVecY(camera_->GetPosition(), camera_->GetTarget());

        // 描画
        ClearDrawScreen();
        MV1DrawModel(modelHandleId_);
        ScreenFlip();
    }
}

void Game::End()
{
    if (modelHandleId_ != -1) MV1DeleteModel(modelHandleId_);
    DxLib_End();
}
