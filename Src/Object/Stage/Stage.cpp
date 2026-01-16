#include "../../Utility/AsoUtility.h"
#include "../Player/Common/Player.h"
#include "../Player/Common/PlayerManager.h"
#include "Stage.h"
#include <cmath>
#include "../../Manager/InputManager.h"

// 静的
Stage* Stage::instance_ = nullptr;

// インスタンス取得
Stage& Stage::GetInstance()
{
	if (instance_ == nullptr)
	{
		CreateInstance();
	}
	return *instance_;
}

void Stage::CreateInstance()
{
	if (instance_ == nullptr)
	{
		instance_ = new Stage();
	}
}

// 初期化
void Stage::Init()
{
	// モデルの読み込み
	modelId_ = MV1LoadModel("Data/Model/Stage/stage.mv1");

	// スカイドームモデルの読み込み
	skyModelId_ = MV1LoadModel("Data/Model/Stage/skydome.mv1");

	// ステージの位置・角度・スケール設定
	pos_ = DEFAULT_POS;
	angle_ = AsoUtility::VECTOR_ZERO;
	scale_ = DEFAULT_SCALE;

	// スカイドームの設定
	skyPos_ = { 0.0f, 250.0f, 0.0f };
	skyAngle_ = AsoUtility::VECTOR_ZERO;
	skyScale_ = { 50.0f, 50.0f, 50.0f };

	// コライダーの設定	
	collider_.center = pos_;
	collider_.radius = COLLIDER_RADIUS;
	collider_.yMin = pos_.y;
	collider_.yMax = pos_.y + COLLIDER_YMAX_OFFSET;

	// 回転系
	angularVelocity_ = AsoUtility::VECTOR_ZERO;
	momentOfInertia_ = MOMENT_OF_INERTIA;
	dampingFactor_ = 0.10f;         // 少し強めの減衰
	restitutionFactor_ = 0.25f;     // 復元トルク強め

	// 無人グレース初期化
	lastOnStageTimeMs_ = GetNowCount();
}

// 更新
void Stage::Update()
{
	// 自然減衰（角速度→角度）
	angle_.x += angularVelocity_.x;
	angle_.y += angularVelocity_.y;
	angle_.z += angularVelocity_.z;

	angularVelocity_.x *= (1.0f - dampingFactor_);
	angularVelocity_.y *= (1.0f - dampingFactor_);
	angularVelocity_.z *= (1.0f - dampingFactor_);
}

// 描画
void Stage::Draw()
{
	MV1SetPosition(modelId_, pos_);
	MV1SetRotationXYZ(modelId_, angle_);
	MV1SetScale(modelId_, scale_);
	MV1DrawModel(modelId_);

	// スカイドームの描画
	MV1SetPosition(skyModelId_, skyPos_);
	MV1SetRotationXYZ(skyModelId_, skyAngle_);
	MV1SetScale(skyModelId_, skyScale_);
	MV1DrawModel(skyModelId_);
}

// 破棄
void Stage::Release()
{
	if (modelId_ != -1)
	{
		MV1DeleteModel(modelId_);
		modelId_ = -1;
	}

	if (skyModelId_ != -1)
	{
		MV1DeleteModel(skyModelId_);
		skyModelId_ = -1;
	}
}

// プレイヤーの位置に応じてステージを傾斜
void Stage::UpdateTilt(const std::vector<Player*>& players)
{
	const int nowMs = GetNowCount();

	// 端マージン付きで在ステージ判定
	bool anyPlayerOnStage = false;
	for (auto p : players)
	{
		if (IsPlayerOnStage(p->GetPos(), ON_STAGE_MARGIN))
		{
			anyPlayerOnStage = true;
			break;
		}
	}

	lastOnStageTimeMs_ = nowMs;

	// 在ステージプレイヤーの重心
	float totalWeight = 0.0f;
	float weightedX = 0.0f;
	float weightedZ = 0.0f;

	for (auto p : players)
	{
		// 死んでいるとき、落下しているときは除く
		if (!(p->IsAlive() && !p->IsFalling() && IsPlayerOnStage(p->GetPos(), ON_STAGE_MARGIN)))
			continue;

		float weight = p->GetWeight();
		VECTOR rel = VSub(p->GetPos(), this->pos_);
		rel.y = 0.0f;

		// 円外に出ないようクランプ
		float dist = rel.x * rel.x + rel.z * rel.z;
		if (dist > 0.0f)
		{
			float d = std::sqrt(dist);
			if (d > collider_.radius)
			{
				float s = collider_.radius / d;
				rel.x *= s;
				rel.z *= s;
			}
		}

		weightedX += rel.x * weight;
		weightedZ += rel.z * weight;
		totalWeight += weight;
	}

	// 重さゼロなら何もしない
	if (totalWeight <= 0.0f) return;

	float centerX = weightedX / totalWeight;
	float centerZ = weightedZ / totalWeight;

	// プレイヤー重心に基づくトルク
	VECTOR torque = AsoUtility::VECTOR_ZERO;
	torque.x = centerZ * totalWeight * GRAVITY;
	torque.z = -centerX * totalWeight * GRAVITY;

	// 復元トルク
	VECTOR restoringTorque = AsoUtility::VECTOR_ZERO;
	restoringTorque.x = -angle_.x * restitutionFactor_;
	restoringTorque.z = -angle_.z * restitutionFactor_;

	// 合計トルク
	VECTOR totalTorque = VAdd(torque, restoringTorque);

	// 角加速度
	VECTOR angularAcceleration = VScale(totalTorque, 1.0f / momentOfInertia_);

	// 角速度更新
	float deltaTime = 1.0f / 60.0f;
	angularVelocity_ = VAdd(angularVelocity_, VScale(angularAcceleration, deltaTime));

	// 角速度の上限（急激な傾き防止）
	const float maxAngularVel = AsoUtility::Deg2RadF(2.0f); // 1フレームで最大2度
	angularVelocity_.x = std::fmax(std::fmin(angularVelocity_.x, maxAngularVel), -maxAngularVel);
	angularVelocity_.z = std::fmax(std::fmin(angularVelocity_.z, maxAngularVel), -maxAngularVel);

	// 減衰
	angularVelocity_ = VScale(angularVelocity_, dampingFactor_);

	// 角度更新
	angle_ = VAdd(angle_, VScale(angularVelocity_, deltaTime));

	// 角度の上限（詰み防止・自然な傾き）
	const float maxTilt = AsoUtility::Deg2RadF(30.0f); // 30度まで
	angle_.x = std::fmax(std::fmin(angle_.x, maxTilt), -maxTilt);
	angle_.z = std::fmax(std::fmin(angle_.z, maxTilt), -maxTilt);

	// 角度が上限に達したら角速度をリセット
	if (std::abs(angle_.x) >= maxTilt) angularVelocity_.x = 0.0f;
	if (std::abs(angle_.z) >= maxTilt) angularVelocity_.z = 0.0f;

	// 反映
	MV1SetRotationXYZ(modelId_, angle_);

	// コリジョン更新
	MV1RefreshCollInfo(modelId_);

	// Y回転は使わない
	angle_.y = 0.0f;
}

bool Stage::IsPlayerOnStage(const VECTOR& playerPos) const
{
	return IsPlayerOnStage(playerPos, 0.0f);
}

bool Stage::IsPlayerOnStage(const VECTOR& playerPos, float margin) const
{
	VECTOR relationPos = VSub(playerPos, this->pos_);
	relationPos.y = 0.0f;	
	float distSq = relationPos.x * relationPos.x + relationPos.z * relationPos.z;

	const float r = collider_.radius + margin;
	return distSq <= (r * r);
}

VECTOR Stage::GetStageNormal() const
{
	VECTOR up = { 0.0f, 1.0f, 0.0f };
	VECTOR v = VTransform(up, MGetRotX(angle_.x));
	v = VTransform(v, MGetRotZ(angle_.z));
	v = VTransform(v, MGetRotY(angle_.y));
	return v;
}
