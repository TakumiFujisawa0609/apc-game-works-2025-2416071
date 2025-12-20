#pragma once
#include <chrono>
#include <vector>
class SceneBase;
class Fader;
class Camera;
class PauseOverlay;


class SceneManager
{

public:

	// 背景色
	static constexpr int BACKGROUND_COLOR_R = 0;
	static constexpr int BACKGROUND_COLOR_G = 139;
	static constexpr int BACKGROUND_COLOR_B = 139;

	// ディレクションライトの方向
	static constexpr VECTOR LIGHT_DIRECTION = { 0.3f, -0.7f, 0.8f };

	// シーン管理用
	enum class SCENE_ID
	{
		NONE,
		TITLE,
		MANUAL,
		PLAYERNUMBERSELECT,
		CHARASELECT,
		GAME,
		RANKING,
		RESULT,
	};

	// インスタンス生成
	static void CreateInstance(void);

	// インスタンス取得
	static SceneManager& GetInstance(void);

	// 初期化
	void Init(void);

	// 3D初期化
	void Init3D(void);

	// 更新
	void Update(void);

	// 描画
	void Draw(void);

	// リソース解放
	void Destroy(void);

	// 遷移
	void ChangeScene(SCENE_ID nextId);

	// 現在シーンID
	SCENE_ID GetSceneID(void) const { return sceneId_; }

	// ΔTime
	float GetDeltaTime(void) const { return deltaTime_; }

	// カメラ
	Camera* GetCamera(void) const { return camera_; }

	bool IsPaused() const { return paused_; }

private:

	// 単一インスタンス
	static SceneManager* instance_;

	SCENE_ID sceneId_;
	SCENE_ID waitSceneId_;

	// フェード
	Fader* fader_;

	// 現在のシーン
	SceneBase* scene_;

	// カメラ
	Camera* camera_;

	// 遷移中フラグ
	bool isSceneChanging_;

	// デルタタイム
	std::chrono::system_clock::time_point preTime_;
	float deltaTime_;

	// コンストラクタは private
	SceneManager(void);

	// コピー禁止
	SceneManager(const SceneManager& instance) = default;

	// デストラクタ
	~SceneManager(void) = default;

	// ΔTime再設定
	void ResetDeltaTime(void);

	// シーン切替実行
	void DoChangeScene(SCENE_ID sceneId);

	// フェード制御
	void Fade(void);

	// プレイヤー数
	int playerNum_;

	// キャラセレで最後に選ばれたタイプ一覧（ID順）
	std::vector<int> selectedPlayerNums_;

public:
	void SetPlayerNum(int num) { playerNum_ = num; }
	int GetPlayerNum(void) const { return playerNum_; }

	// セーブ: キャラセレ（ID順）
	void SetSelectedPlayerNums(const std::vector<int>& types) { selectedPlayerNums_ = types; }
	const std::vector<int>& GetSelectedPlayerNums() const { return selectedPlayerNums_; }

	// 追加: 直近ゲームで実際に出場したタイプ一覧（ID順、GameSceneが設定）
	void SetLastSpawnedTypes(const std::vector<int>& types) { lastSpawnedTypes_ = types; }
	const std::vector<int>& GetLastSpawnedTypes() const { return lastSpawnedTypes_; }

private:
	// 追加: 直近ゲームのスポーンタイプ（Resultで使用）
	std::vector<int> lastSpawnedTypes_;

	bool paused_ = false;
	PauseOverlay* pauseOverlay_ = nullptr;
};