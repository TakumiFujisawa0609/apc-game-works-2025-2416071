#include "Stage.h"
#include "../Player/Common/Player.h"
#include "../Player/Common/PlayerManager.h"
#include "../../Utility/AsoUtility.h"
#include <cmath>
#include <float.h>
#include <algorithm>

// 静的メンバ変数の定義
Stage* Stage::instance_ = nullptr;

// コンストラクタ・デストラクタ
Stage::Stage() {}
Stage::~Stage() {}

// インスタンス管理
void Stage::CreateInstance() 
{
	if (instance_ == nullptr) instance_ = new Stage();
}

Stage& Stage::GetInstance()
{
	if (!instance_) Stage::CreateInstance();
	return *instance_;
}

// 基本処理
void Stage::Init()
{
	// モデルの読み込み
	modelId_ = MV1LoadModel("Data/Model/Stage/Stage.mv1");

	// モデルの位置・角度・スケールの設定
	pos_ = { 0.0f,-1500.f,0.0f };
	angle_ = { 0.0f,0.0f,0.0f };
	scale_ = { 2.0f,2.0f,2.0f };

	// コライダーの設定
	collider_.center = pos_;
	collider_.radius = 600.0f;
	collider_.yMin = pos_.y;			// ステージの下端
	collider_.yMax = pos_.y + 50000.0f; // ステージの上端

	// 物理制御の初期化
	angularVelocity_ = AsoUtility::VECTOR_ZERO;
	momentOfInertia_ = (float)24000;		// 慣性
	dampingFactor_ = 0.4f;					// 角速度の減衰率
	restitutionFactor_ = 3000.0f;			// 反発

	// 衝突情報の初期化
	MV1SetupCollInfo(modelId_, -1, 32, 32, 32);
}

void Stage::Update()
{
}

void Stage::Draw(const VECTOR& pos)
{
	// モデルの描画
	MV1SetPosition(modelId_, pos_);
	MV1SetRotationXYZ(modelId_, angle_);
	MV1SetScale(modelId_, scale_);
	MV1DrawModel(modelId_);

	// デバッグ用に角度を表示
	DrawFormatString(0, 400, GetColor(130, 255, 130), "Stage Angle: (%.2f, %.2f, %.2f)", AsoUtility::Rad2DegF(angle_.x), AsoUtility::Rad2DegF(angle_.y), AsoUtility::Rad2DegF(angle_.z));
	DrawSphere3D(collider_.center, collider_.radius, 16, GetColor(255, 0, 0), GetColor(255, 0, 0), FALSE);
}

void Stage::Release()
{
	// モデルの解放
	if (modelId_ != -1) {
		MV1DeleteModel(modelId_);
		modelId_ = -1;
	}
}

// プレイヤーの位置に応じてステージを傾ける
void Stage::UpdateTilt(const std::vector<Player*>& players)
{
	// プレイヤーの機能実装まで傾けない
	//return;

	// プレイヤーの情報がなければ傾けない
	if (players.empty()) return;

	// プレイヤーの重さを

	// プレイヤーの平均位置を計算
	float totalWeight = 0.0f;
	float weightedX = 0.0f;
	float weightedZ = 0.0f;

	// あらぶり対策
	bool anyPlayerOnStage = false;
	VECTOR totalPlayerPos = AsoUtility::VECTOR_ZERO;

	for (auto p : players)
	{
		if (IsPlayerOnStage(p->GetPos()))
		{
			anyPlayerOnStage = true;
			totalPlayerPos = VAdd(totalPlayerPos, p->GetPos());
		}
	}
	if (!anyPlayerOnStage)
	{
		// プレイヤーが誰もステージ上にいない場合

		// 徐々に中央に戻すことで、荒ぶりを止めつつ自然な停止を表現
		angle_.x = AsoUtility::Lerp(angle_.x, 0.0f, 0.05f); // 0.05fは戻る速さ
		angle_.z = AsoUtility::Lerp(angle_.z, 0.0f, 0.05f);

		// Y軸の角度は変えない
		angle_.y = 0.0f;

		return; // ★ 物理計算をスキップして終了
	}

	for (auto p : players)
	{
		float w = p->GetWeight();
		VECTOR pos = p->GetPos();
		weightedX += (pos.x - this->pos_.x) * w;
		weightedZ += (pos.z - this->pos_.z) * w;
		totalWeight += w;

	}

	// 重さが0以下ならreturn
	if (totalWeight <= 0.0f) return;

	// 中心位置
	float centerX = weightedX / totalWeight;
	float centerZ = weightedZ / totalWeight;

	// プレイヤーの重さによる力の計算
	float totalMass = totalWeight; // 重さの総和を質量とみなす
	const float GRAVITY = 9.81f; // 重力加速度

	// プレイヤーの重さによる力(プレイヤーの合計質量とステージの中心からの距離で計算)
	// T = r * F
	VECTOR torque = AsoUtility::VECTOR_ZERO;

	// X軸周りの力
	torque.x = centerZ * totalMass * GRAVITY; // Z方向の位置が遠いほどX軸周りの力が大きい

	// Z軸周りの力
	torque.z = -centerX * totalMass * GRAVITY; // X方向の位置が遠いほどZ軸周りの力が大きい


	// ステージの傾きを元に戻そうとする力
	VECTOR restoringTorque = AsoUtility::VECTOR_ZERO;
	restoringTorque.x = -angle_.x * restitutionFactor_; // X軸周りの復元力
	restoringTorque.z = -angle_.z * restitutionFactor_; // Z軸周りの復元力

	// すべての力を合計
	VECTOR totalTorque = AsoUtility::VECTOR_ZERO;
	totalTorque = VAdd(torque, restoringTorque);

	// 角加速度の計算
	VECTOR angularAcceleration = VScale(totalTorque, 1.0f / momentOfInertia_);

	//　角速度の更新
	float deltaTime = 1.0f / 60.0f; // フレーム時間（60FPS想定）
	angularVelocity_ = VAdd(angularVelocity_, VScale(angularAcceleration, deltaTime));

	// 減衰の適用
	angularVelocity_ = VScale(angularVelocity_, dampingFactor_);

	// 角度の更新
	angle_ = VAdd(angle_, VScale(angularVelocity_, deltaTime));

	// 最大角の制限
	const float maxTilt = AsoUtility::Deg2RadF(45.0f); // 最大傾き45度
	angle_.x = std::fmax(std::fmin(angle_.x, maxTilt), -maxTilt);
	angle_.z = std::fmax(std::fmin(angle_.z, maxTilt), -maxTilt);

	// ステージの角度をモデルに反映
	MV1SetRotationXYZ(modelId_, angle_);

	// 衝突情報の更新
	MV1RefreshCollInfo(modelId_);

	// Y軸の角度は変えない
	angle_.y = 0.0f;
}

float Stage::GetGroundHeight(const VECTOR& pos, float capsuleRadius, float capsuleHalfHeight) const
{
	VECTOR p1 = VGet(pos.x, pos.y + capsuleHalfHeight, pos.z);
	VECTOR p2 = VGet(pos.x, pos.y - capsuleHalfHeight, pos.z);

	MV1_COLL_RESULT_POLY_DIM result = MV1CollCheck_Capsule(modelId_, -1, p1, p2, capsuleRadius);

	float maxY = -FLT_MAX;

	if (result.HitNum > 0 && result.Dim != nullptr)
	{
			// result.Dim[i] がポインタ配列の場合に備えて -> に修正
		for (int i = 0; i < result.HitNum; i++)
		{
			if (result.Dim[i].HitFlag)
			{
				float hitY = result.Dim[i].Position->y;
				if (hitY > maxY)
				{
					maxY = hitY;
				}
			}
		}
	}

	MV1CollResultPolyDimTerminate(result);

	if (maxY == -FLT_MAX)
	{
		return -9999.0f;
	}

	return maxY;
}




bool Stage::IsPlayerOnStage(const VECTOR& playerPos) const
{
	// ステージの中心からプレイヤーまでの水平位置を計算
	VECTOR relationPos = VSub(playerPos, this->pos_);

	//Y軸は無視で、水平面のみで距離を計算
	relationPos.y = 0.0f;
	float dist = relationPos.x * relationPos.x + relationPos.z * relationPos.z;

	// 距離の二乗が範囲の二乗以内ならステージ内
	return dist <= (collider_.radius * collider_.radius);
}