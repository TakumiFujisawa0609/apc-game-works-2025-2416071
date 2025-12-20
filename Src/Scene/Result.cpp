#include "Result.h"
#include "../Object/Player/Common/PlayerManager.h"
#include "../Application.h"
#include "../Utility/AsoUtility.h"
#include "../Manager/InputManager.h"
#include "../Manager/SceneManager.h"

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
}

void Result::Update(void)
{
	// 入力
	InputManager& ins = InputManager::GetInstance();

	// タイトルへ戻る
	if (ins.IsNew(KEY_INPUT_RETURN) || ins.IsPadBtnNew(InputManager::JOYPAD_NO::PAD1, InputManager::JOYPAD_BTN::RIGHT))
	{
		SceneManager::GetInstance().ChangeScene(SceneManager::SCENE_ID::TITLE);
	}
}

void Result::Draw(void)
{
	// 結果モデル用の簡易カメラを固定配置
	{
		VECTOR camPos = VGet(0.0f, 250.0f, -800.0f);
		float pitch = 20.0f * DX_PI_F / 180.0f;
		SetCameraPositionAndAngle(camPos, pitch, 0.0f, 0.0f);
	}

	// 背景
	DrawRotaGraph(Application::SCREEN_SIZE_X / 2, Application::SCREEN_SIZE_Y / 2, 0.75, 0.0, bgImg_, TRUE);

	// モデル描画
	for (size_t i = 0; i < modelHandles_.size(); ++i)
	{
		int h = modelHandles_[i];
		if (h == -1) continue;
		MV1SetScale(h, VGet(modelScale_, modelScale_, modelScale_));
		MV1SetRotationXYZ(h, VGet(0.0f, 0.0f, 0.0f));
		MV1SetPosition(h, modelPositions_[i]);
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

	DrawString(40, 70, "Enter / PAD1-B: タイトルへ", GetColor(220, 220, 220));
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