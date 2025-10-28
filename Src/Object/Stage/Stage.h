#pragma once
#include <DxLib.h>
#include <vector>
#include <algorithm>

class Player;

struct CylinderCollider
{
	VECTOR center;	// 中心位置
	float radius;	// 半径
	float yMin;     // Yの最小値
	float yMax;		// Yの最大値
};

class Stage
{
public: 

	// 定数 
	static constexpr VECTOR DEFAULT_POS = { 0.0f,-750.f,0.0f };

	
	static void CreateInstance();
	static Stage& GetInstance();

	// 基本処理
	void Init();
	void Update();
	void Draw();
	void Release();

	// モデルID
	int GetModelId() const { return modelId_; }

	// ステージの位置・角度・スケール
	const VECTOR& GetPos() const { return pos_; }
	const VECTOR& GetAngle() const { return angle_; }
	const VECTOR& GetScale() const { return scale_; }

	// ステージの傾き更新（プレイヤーの位置に応じて傾く）
	const CylinderCollider& GetCollider() const { return collider_; }

	void UpdateTilt(const std::vector<Player*>& players);
	float GetGroundHeight(const VECTOR& pos) const;

	// playerがステージ内にいるか判定
	bool IsPlayerOnStage(const VECTOR& playerPos) const;

private:

	// コンストラクタ・デストラクタ
	Stage();
	~Stage();

	static Stage* instance_;

	int modelId_;			// ステージのモデルID
	VECTOR pos_;			// ステージの位置
	VECTOR angle_;			// ステージの角度
	VECTOR scale_;			// ステージのスケール
	CylinderCollider collider_; // ステージの当たり判定（円柱）


	// 物理的な制御用関数
	VECTOR angularVelocity_; // 角速度
	float momentOfInertia_; // 慣性モーメント
	float dampingFactor_; // 減衰係数
	float restitutionFactor_; // 反発係数

	float maxStageRange_ = 10000.f; // ステージの最大範囲


};