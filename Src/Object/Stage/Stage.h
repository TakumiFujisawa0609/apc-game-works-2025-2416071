#pragma once
#include <DxLib.h>
#include <vector>
#include <algorithm>

class Player;

// 円筒コライダー
struct CylinderCollider
{
	VECTOR center;	// 中心座標
	float radius;	// 半径
	float yMin;		// Yの最小
	float yMax;		// Yの最大
};

class Stage
{
public:

	// 定数
	static constexpr VECTOR DEFAULT_POS = { 0.0f, -2250.0f, 0.0f };			// ステージの初期座標
	static constexpr VECTOR DEFAULT_SCALE = { 3.0f, 3.0f, 3.0f };			// ステージの初期スケール
	static constexpr float COLLIDER_RADIUS = 1000.0f;						// コライダー半径
	static constexpr float COLLIDER_YMAX_OFFSET = 5000.0f;					// コライダーY最大のオフセット
	static constexpr float MOMENT_OF_INERTIA = 10000.0f;						// 慣性モーメント
	static constexpr float DAMPING_FACTOR = 0.05f;							// 減衰

	// 端許容マージン（プレイヤー中心が少しはみ出しても在ステージとみなす幅）
	static constexpr float ON_STAGE_MARGIN = 120.0f; 

	// 無人グレースタイマー（ms）
	static constexpr int NO_PLAYER_GRACE_MS = 700; 

	const float GRAVITY = 9.81f; // 重力

	// インスタンス
	static void CreateInstance();
	static Stage& GetInstance();

	// ライフサイクル
	void Init();
	void Update();
	void Draw();
	void Release();

	// 参照
	int GetModelID() const { return modelId_; }
	const VECTOR& GetPos() const { return pos_; }
	const VECTOR& GetAngle() const { return angle_; }
	const VECTOR& GetScale() const { return scale_; }
	const CylinderCollider& GetCollider() const { return collider_; }

	// ステージ傾斜更新
	void UpdateTilt(const std::vector<Player*>& players);

	// 在ステージ判定（従来・マージンなし）
	bool IsPlayerOnStage(const VECTOR& playerPos) const;

	// 在ステージ判定（マージンあり）
	bool IsPlayerOnStage(const VECTOR& playerPos, float margin) const;

	// 法線
	VECTOR GetStageNormal() const;

private:

	// コンストラクタ/デストラクタ
	Stage() = default;
	~Stage() = default;

	// インスタンス
	static Stage* instance_;

	// モデルID
	int modelId_ = -1;
	// 位置・角度・スケール
	VECTOR pos_;
	VECTOR angle_;
	VECTOR scale_;

	// コライダー
	CylinderCollider collider_;

	// 回転用
	VECTOR angularVelocity_;
	float momentOfInertia_;
	float dampingFactor_;
	float restitutionFactor_;

	float maxStageRange_ = 600.0f;

	// スカイドーム
	int skyModelId_ = -1;
	VECTOR skyPos_;
	VECTOR skyScale_;
	VECTOR skyAngle_;

	// 無人グレース管理
	int lastOnStageTimeMs_ = 0;
};