#pragma once
#include <DxLib.h>
#include <vector>
#include <algorithm>

class Player;

// シリンダー型コライダー
struct CylinderCollider
{
	VECTOR center;	// 中心位置
	float radius;	// 半径
	float yMin;		// Yの最小値
	float yMax;		// Yの最大値
};

class Stage
{
public:

	// 定数
	static constexpr VECTOR DEFAULT_POS = { 0.0f, -2250.0f, 0.0f };			// ステージの初期位置
	static constexpr VECTOR DEFAULT_SCALE = { 3.0f, 3.0f, 3.0f };			// ステージの初期スケール
	static constexpr float COLLIDER_RADIUS = 1000.0f;						// コライダーの半径
	static constexpr float COLLIDER_YMAX_OFFSET = 5000.0f;					// コライダーのY最大値オフセット
	static constexpr float MOMENT_OF_INERTIA = 10000.0f;						// プレイヤーに対してステージの反発力(値が大きいほど傾きにくい)
	static constexpr float DAMPING_FACTOR = 0.05f;							// プレイヤーの方向に傾く速さ

	// インスタンス
	static void CreateInstance();
	static Stage& GetInstance();

	// 基本処理
	void Init();
	void Update();
	void Draw();
	void Release();

	// モデルUD
	int GetModelID() const { return modelId_; }

	// ステージの位置・角度・スケール取得
	const VECTOR& GetPos() const { return pos_; }
	const VECTOR& GetAngle() const { return angle_; }
	const VECTOR& GetScale() const { return scale_; }

	// ステージの傾き更新
	const CylinderCollider& GetCollider() const {return collider_;}

	// プレイヤーの位置に応じてステージを傾ける
	void UpdateTilt(const std::vector<Player*>& players);

	// playerがステージ内にいるかどうか確認
	bool IsPlayerOnStage(const VECTOR& playerPos) const;

	// ステージの傾きを渡す関数
	VECTOR GetStageNormal() const;

private:

	// コンストラクタ
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

	// 物理制御用変数
	VECTOR angularVelocity_;
	float momentOfInertia_;
	float dampingFactor_;
	float restitutionFactor_;

	float maxStageRange_ = 600.0f;


	// スカイドームモデル
	int skyModelId_ = -1;
	// スカイドームの位置・スケール
	VECTOR skyPos_;
	VECTOR skyScale_;
	VECTOR skyAngle_;

};