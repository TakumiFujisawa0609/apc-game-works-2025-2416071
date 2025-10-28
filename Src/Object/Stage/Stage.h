#pragma once
#include <DxLib.h>
#include <vector>
#include <algorithm>

class Player;

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
	static constexpr VECTOR DEFAULT_POS = { 0.0f,-750.0f,0.0f };

	static void CreateInstance();
	static Stage& GetInstance();

	// 基本処理
	void Init();
	void Update();
	void Draw(const VECTOR& pos);
	void Release();

	// モデルID取得
	int GetModelID() const { return modelId_; }

	// ステージの傾きを取得
	const VECTOR& GetPos() const { return pos_; }
	const VECTOR& GetAngle() const { return angle_; }
	const VECTOR& GetScale() const { return scale_; }

	// ステージの傾き更新
	const CylinderCollider& GetCollider() const { return collider_; }

	// 地面の高さ取得
	void UpdateTilt(const std::vector<Player*>& players);

	float GetGroundHeight(const VECTOR& pos, float capsuleRadius, float capsuleHalfHeight) const;

	//float GetGroundHeight(const VECTOR& pos, float capsuleRadius, float capsuleHalfHeight);

	// ステージ上にいるか判定
	// カプセルでの当たり判定
	//float GetGroundHeight(const VECTOR& pos, float capsuleRadius, float capsuleHeight) const;

	// プレイヤーがステージ上にいるか判定
	bool IsPlayerOnStage(const VECTOR& position) const;

private:

	// コンストラクタ・デストラクタ
	Stage();
	~Stage();

	static Stage* instance_;

	int modelId_;				// ステージのモデルID
	VECTOR pos_;				// ステージの位置
	VECTOR angle_;				// ステージの角度
	VECTOR scale_;				// ステージのスケール
	CylinderCollider collider_; // ステージの当たり判定（円柱）


	// 物理的な制御用関数
	VECTOR angularVelocity_;	// 角速度
	float momentOfInertia_;		// 慣性モーメント
	float dampingFactor_;		// 減衰係数
	float restitutionFactor_;	// 反発係数

	float maxStageRange_ = 10000.f; // ステージの最大範囲
};