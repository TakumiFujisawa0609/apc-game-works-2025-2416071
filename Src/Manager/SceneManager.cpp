#include <chrono>
#include <DxLib.h>
#include "../Common/Fader.h"
#include "../Scene/TitleScene.h"
#include "../Scene/GameScene.h"
#include "../Scene/ManualScene.h"
#include "../Scene/PlayerNumScene.h"
#include "../Scene/CharacterSelect.h"
#include "../Scene/Result.h"
#include "Camera.h"
#include "SceneManager.h"
#include "../Common/PauseOverlay.h"
#include "../Manager/InputManager.h"

SceneManager* SceneManager::instance_ = nullptr;

void SceneManager::CreateInstance()
{
	if (instance_ == nullptr)
	{
		instance_ = new SceneManager();
	}
	instance_->Init();
}

SceneManager& SceneManager::GetInstance(void)
{
	return *instance_;
}

void SceneManager::Init(void)
{

	sceneId_ = SCENE_ID::TITLE;
	waitSceneId_ = SCENE_ID::NONE;

	// フェード機能の初期化
	fader_ = new Fader();
	fader_->Init();

	// カメラ
	camera_ = new Camera();
	camera_->Init();

	scene_ = new TitleScene();
	scene_->Init();

	isSceneChanging_ = false;

	// デルタタイム
	preTime_ = std::chrono::system_clock::now();

	// 3D設定
	Init3D();

	// 初期シーン設定
	DoChangeScene(SCENE_ID::TITLE);

	// ポーズオーバーレイ初期化
	pauseOverlay_ = new PauseOverlay();
	pauseOverlay_->Init();
	paused_ = false;
}

void SceneManager::Init3D(void)
{

	// 背景色
	SetBackgroundColor(
		BACKGROUND_COLOR_R,
		BACKGROUND_COLOR_G,
		BACKGROUND_COLOR_B);

	// Zバッファ
	SetUseZBuffer3D(true);
	SetWriteZBuffer3D(true);

	// バックカリング
	SetUseBackCulling(true);

	// ライティング設定
	SetUseLighting(true);

	// 光源設定
	ChangeLightTypeDir({ 0.00f, -1.00f, 1.00f });

}

void SceneManager::Update(void)
{

	if (scene_ == nullptr)
	{
		return;
	}

	// デルタタイム更新（ゲーム内時間の算出は維持。実処理は pause で止める）
	auto nowTime = std::chrono::system_clock::now();
	deltaTime_ = static_cast<float>(
		std::chrono::duration_cast<std::chrono::nanoseconds>(nowTime - preTime_).count() / 1000000000.0);
	preTime_ = nowTime;

	// ESC でポーズトグル（フェード中は無視）
	auto& ins = InputManager::GetInstance();
 	if (!isSceneChanging_ && ins.IsTrgDown(KEY_INPUT_ESCAPE)) {
		paused_ = !paused_;
		// 新規ポーズ開始時にオーバーレイ初期化
		if (paused_) {
			pauseOverlay_->Init();
		}
	}

	if (paused_)
	{
		// ポーズ中はフェード・シーン更新・カメラ更新を止める
		bool reqResume = false, reqGoTitle = false;
		pauseOverlay_->Update(reqResume, reqGoTitle);

		if (reqGoTitle) {
			paused_ = false;
			ChangeScene(SCENE_ID::TITLE);
			return;
		}
		if (reqResume) {
			paused_ = false;
		}
		return; // 以降のシーン更新を行わない
	}

	// フェード更新・シーン更新
	fader_->Update();
	if (isSceneChanging_)
	{
		// フェード状態の遷移処理
		Fade();
	}
	else
	{
		// 各シーンの更新処理
		scene_->Update();
	}

	// カメラ更新
	camera_->Update();

	// ミニカメラ
	//miniCamera_->Update();

}

void SceneManager::Draw(void)
{

	//miniCamera_->DrawScreen();

	// 描画先の設定
	SetDrawScreen(DX_SCREEN_BACK);

	// 画面クリア
	ClearDrawScreen();

	// カメラ設定
	camera_->SetBeforeDraw();

	// シーン描画
	scene_->Draw();

	// ミニカメラ
	//miniCamera_->Draw();

	// カメラデバッグ
	camera_->DrawDebug();

	// フェード
	fader_->Draw();

	// ポーズ中は最後にオーバーレイ描画
	if (paused_ && pauseOverlay_) {
		pauseOverlay_->Draw();
	}
}

void SceneManager::Destroy(void)
{

	// シーンの破棄
	if (scene_) {
		scene_->Release();
		delete scene_;
		scene_ = nullptr;
	}

	// フェードの破棄
	delete fader_;
	fader_ = nullptr;

	// カメラ破棄
	if (camera_) {
		camera_->Release();
		delete camera_;
		camera_ = nullptr;
	}

	// ポーズオーバーレイ破棄
	delete pauseOverlay_;
	pauseOverlay_ = nullptr;

	// インスタンス破棄
	delete instance_;
	instance_ = nullptr;

}

void SceneManager::ChangeScene(SCENE_ID nextId)
{

	// フェード中でも依頼が来た場合は通常通りフェードして遷移
	waitSceneId_ = nextId;

	// フェードアウト（暗転）開始
	fader_->SetFade(Fader::STATE::FADE_OUT);
	isSceneChanging_ = true;
}


SceneManager::SceneManager(void)
{

	sceneId_ = SCENE_ID::NONE;
	waitSceneId_ = SCENE_ID::NONE;

	scene_ = nullptr;
	fader_ = nullptr;

	isSceneChanging_ = false;

	// デルタタイム
	deltaTime_ = 1.0f / 60.0f;

	camera_ = nullptr;

	// ポーズ管理
	paused_ = false;
	pauseOverlay_ = nullptr;
	

}

void SceneManager::ResetDeltaTime(void)
{
	deltaTime_ = 0.016f;
	preTime_ = std::chrono::system_clock::now();
}

void SceneManager::DoChangeScene(SCENE_ID sceneId)
{

	// シーン切替
	sceneId_ = sceneId;

	// 現在のシーン破棄
	if (scene_ != nullptr)
	{
		scene_->Release();
		delete scene_;
		scene_ = nullptr;
	}

	switch (sceneId_)
	{
		// タイトルシーン
	case SCENE_ID::TITLE:
		scene_ = new TitleScene();
		break;
		// マニュアルシーン　　タイトル ← マニュアル ← 人数選択
	case SCENE_ID::MANUAL:
		scene_ = new ManualScene();
		break;
		// 人数選択シーン		   人数選択 ← キャラ選択
	case SCENE_ID::PLAYERNUMBERSELECT:
		scene_ = new PlayerNumScene();
		break;
		// キャラ選択シーン		   キャラ選択 ← ゲーム
	case SCENE_ID::CHARASELECT:
		scene_ = new CharacterSelect();
		break;
		// ゲームシーン
	case SCENE_ID::GAME:
		scene_ = new GameScene();
		break;
		// ランキング（未実装）
	case SCENE_ID::RANKING:
		//scene_ = new RankingScene();
		break;
		// リザルトシーン
	case SCENE_ID::RESULT:
		scene_ = new Result();
		break;
	}

	// 新しいシーンの初期化
	if (scene_) {
		scene_->Init();
	}

	ResetDeltaTime();

	waitSceneId_ = SCENE_ID::NONE;

}

void SceneManager::Fade(void)
{

	Fader::STATE fState = fader_->GetState();
	switch (fState)
	{
	case Fader::STATE::FADE_IN:
		// 明転中
		if (fader_->IsEnd())
		{
			// 明転が終わったらフェードを停止
			fader_->SetFade(Fader::STATE::NONE);
			isSceneChanging_ = false;
		}
		break;
	case Fader::STATE::FADE_OUT:
		// 暗転中
		if (fader_->IsEnd())
		{
			// 完全に暗転したら次のシーンへ
			DoChangeScene(waitSceneId_);
			// 暗転後、明転へ
			fader_->SetFade(Fader::STATE::FADE_IN);
		}
		break;
	}

}