#include "Result.h"
#include "../Object/Player/Common/PlayerManager.h"
#include "../Application.h"
#include "../Utility/AsoUtility.h"
#include "../Manager/InputManager.h"
#include "../Manager/SceneManager.h"
#include "../Manager/SoundManager.h"

Result::Result(void)
{
}

Result::~Result(void)
{
}

void Result::Init(void)
{
	// リセット
	playerWinID_ = -1;
	for (int i = 0; i < 4; ++i) {
		playerScores_[i] = 0;
	}

	// プレイ人数
	singlePlayer_ = (SceneManager::GetInstance().GetPlayerNum() == 1);

	// 勝者（-1 の場合はゲームオーバー）
	playerWinID_ = PlayerManager::GetInstance().GetWinnerID();

	// モデル準備
	BuildModelsForResult();

	// 背景色設定
	bgImg_ = LoadGraph("Data/Image/Background.png");

	// ボタン
	//buttonImg_ = LoadGraph("data/Image/button/xbox_a_outline.png");

	// フェード/演出用初期化
	fadeTimerMs_ = 0;

	// ソロ負け時のBGM
	SoundManager& snd = SoundManager::GetInstance();
	//snd.StopAllBGM();                 // まず全停止
	if (singlePlayer_ && playerWinID_ != 0) 
	{
		// 負けBGM（共通の RESULT を使用。個別に分ける場合は SoundManager 側に曲を追加）
		snd.PlayBGM(BGM_ID::LOSE, true);
		snd.SetBGMVolume(75);        // 必要なら音量調整（0～255）
	}
	else 
	{
		// それ以外（勝ち/マルチ勝者表示など）の場合にも同じ曲で良ければ再生
		snd.PlayBGM(BGM_ID::WIN, true);
		snd.SetBGMVolume(75);
	}
}

void Result::Update(void)
{
	// 経過（揺れ/点滅用）
	int nextFade = fadeTimerMs_ + 16;
	fadeTimerMs_ = (nextFade < 1000) ? nextFade : 1000; // 約1秒で最大（ビネット等で使用）
	const float t = GetNowCount() * 0.001f;
	cameraBobPhase_ = t;
	promptBlinkPhase_ = t;

	// 入力
	InputManager& ins = InputManager::GetInstance();

	// タイトルへ戻る（決定/遷移は A(DOWN)）
	if (ins.IsNew(KEY_INPUT_RETURN) || ins.IsPadBtnNew(InputManager::JOYPAD_NO::PAD1, InputManager::JOYPAD_BTN::DOWN))
	{
		SceneManager::GetInstance().ChangeScene(SceneManager::SCENE_ID::TITLE);
		SoundManager::GetInstance().PlaySE(SE_ID::MOVE2);
		SoundManager::GetInstance().StopAllBGM();
	}
}

void Result::Draw(void)
{
	// 結果モデル用の簡易カメラを固定配置＋微揺れ
	{
		const float bob = 8.0f * sinf(cameraBobPhase_ * 0.8f);
		const float yaw = 0.02f * sinf(cameraBobPhase_ * 0.4f);
		VECTOR camPos = VGet(0.0f, 250.0f + bob, -800.0f);
		float pitch = (20.0f * DX_PI_F / 180.0f);
		SetCameraPositionAndAngle(camPos, pitch, yaw, 0.0f);
	}

	// フェード係数（0.0～1.0）
	const float rawAlpha = fadeTimerMs_ / 1000.0f;
	const float fadeAlpha = (rawAlpha < 1.0f) ? rawAlpha : 1.0f;

	// 背景（不透明）
	DrawRotaGraph(Application::SCREEN_SIZE_X / 2, Application::SCREEN_SIZE_Y / 2, 0.75, 0.0, bgImg_, TRUE);

	// さりげないビネット（周辺減光はフェードに合わせて強度変更）
	{
		int vignetteAlpha = static_cast<int>(fadeAlpha * 90);
		SetDrawBlendMode(DX_BLENDMODE_ALPHA, vignetteAlpha);
		DrawBox(0, 0, Application::SCREEN_SIZE_X, Application::SCREEN_SIZE_Y, GetColor(0, 0, 0), TRUE);
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	}

	// モデル描画（正面固定＋上下揺れ）
	const float modelBobAmp = 12.0f;              // 揺れ幅（必要なら調整）
	const float modelBob = modelBobAmp * sinf(cameraBobPhase_ * 1.6f); // 揺れ速度

	for (size_t i = 0; i < modelHandles_.size(); ++i)
	{
		int h = modelHandles_[i];
		if (h == -1) continue;

		// スケール
		MV1SetScale(h, VGet(modelScale_, modelScale_, modelScale_));

		// 向きを正面に固定
		MV1SetRotationXYZ(h, VGet(0.0f, 0.0f, 0.0f));

		// ベース位置に上下揺れ（Yのみ加算）
		VECTOR pos = modelPositions_[i];
		pos.y += modelBob;
		MV1SetPosition(h, pos);

		MV1DrawModel(h);
	}

	// テキスト
	if (singlePlayer_)
	{
		if (playerWinID_ == 0) {
			DrawString(40, 40, "YOU WIN!", GetColor(255, 255, 120));
		}
		else {
			DrawString(40, 40, "GAME OVER", GetColor(255, 120, 120));
		}
	}
	else
	{
		if (playerWinID_ >= 0) {
			DrawFormatString(40, 40, GetColor(255, 255, 120), "Winner: Player %d", playerWinID_ + 1);
		}
	}

	// ボタン描画
	{
		const int btnX = Application::SCREEN_SIZE_X - 100;
		const int btnY = Application::SCREEN_SIZE_Y - 80;
		DrawRotaGraph(btnX, btnY, 1.0, 0.0, buttonImg_, TRUE);
		// 点滅（フェードイン後、約0.5秒周期で点滅）
		const float blinkCycle = 0.5f; // 秒
		const float blinkPhase = fmodf(promptBlinkPhase_, blinkCycle) / blinkCycle; // 0.0～1.0
		const float blinkAlpha = (blinkPhase < 0.5f) ? (blinkPhase * 2.0f) : (1.0f - (blinkPhase - 0.5f) * 2.0f);
		const float finalAlpha = fadeAlpha * blinkAlpha;
		int alphaInt = static_cast<int>(finalAlpha * 255.0f);
		if (alphaInt > 255) alphaInt = 255;
		if (alphaInt < 0) alphaInt = 0;
		SetDrawBlendMode(DX_BLENDMODE_ALPHA, alphaInt);
		DrawString(btnX - 40, btnY + 30, "Aでタイトルへ", GetColor(255, 255, 255));
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	}
}

void Result::Release(void)
{
	// モデル破棄
	for (int h : modelHandles_) {
		if (h != -1) MV1DeleteModel(h);
	}
	modelHandles_.clear();
	modelPositions_.clear();

	DeleteGraph(bgImg_);
	DeleteGraph(buttonImg_);
}

// モデルの構築（表示するタイプを選んでロード・配置）
void Result::BuildModelsForResult()
{
	modelHandles_.clear();
	modelPositions_.clear();

	const auto& spawned = SceneManager::GetInstance().GetLastSpawnedTypes();

	auto loadByTypeIndex = [](int typeIdx)->int {
		if (typeIdx < 0) typeIdx = 0;
		if (typeIdx > 3) typeIdx = 3;
		const char* path = PlayerManager::GetModelPathForType(static_cast<PlayerType>(typeIdx));
		if (!path) return -1;
		return MV1LoadModel(path);
		};

	if (singlePlayer_)
	{
		// 1P: 勝ち→人間(0)のみ / 負け→CPU3体（1,2,3）
		if (playerWinID_ == 0)
		{
			int type0 = (!spawned.empty()) ? spawned[0] : 0;
			int h0 = loadByTypeIndex(type0);
			modelHandles_.push_back(h0);
			modelPositions_.push_back(VGet(0.0f, 0.0f, 0.0f));
		}
		else
		{
			// CPUはID:1,2,3 を想定（GameScene で 4 体生成している想定）
			for (int id = 1; id <= 3; ++id)
			{
				int typeIdx = (id < (int)spawned.size()) ? spawned[id] : id % 4;
				int h = loadByTypeIndex(typeIdx);
				modelHandles_.push_back(h);

				// 横並び配置
				const float offset = 220.0f;
				float x = (id - 2) * offset; // id=1->-220, id=2->0, id=3->+220
				modelPositions_.push_back(VGet(x, 0.0f, 0.0f));
			}
		}
	}
	else
	{
		// 複数人: 勝者のみ
		if (playerWinID_ >= 0 && playerWinID_ < (int)spawned.size())
		{
			int typeWin = spawned[playerWinID_];
			int hw = loadByTypeIndex(typeWin);
			modelHandles_.push_back(hw);
			modelPositions_.push_back(VGet(0.0f, 0.0f, 0.0f));
		}
	}
}