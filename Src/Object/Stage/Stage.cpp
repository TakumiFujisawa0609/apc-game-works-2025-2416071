#include "../../Utility/AsoUtility.h"
#include "../Player/Common/Player.h"
#include "../Player/Common/PlayerManager.h"
#include "Stage.h"
#include <cmath>
#include "../../Manager/InputManager.h"

// 静的メンバ変数の初期化
Stage* Stage::instance_ = nullptr;

// インスタンス生成・取得
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

	// ステージの位置・角度・スケール初期化
	pos_ = DEFAULT_POS;
	angle_ = AsoUtility::VECTOR_ZERO;
	scale_ = DEFAULT_SCALE;

	// スカイドームモデルの初期化
	skyPos_ = { 0.0f, 250.0f, 0.0f };
	skyAngle_ = AsoUtility::VECTOR_ZERO;
	skyScale_ = { 50.0f, 50.0f, 50.0f };

	// コライダーの初期化	
	collider_.center = pos_;						// コライダーの中心位置をステージの位置に設定
	collider_.radius = COLLIDER_RADIUS;				// コライダーの半径を設定
	collider_.yMin = pos_.y;						// コライダーのY最小値を設定
	collider_.yMax = pos_.y + COLLIDER_YMAX_OFFSET;	// コライダーのY最大値を設定

	// 物理的な傾きの初期化
	angularVelocity_ = AsoUtility::VECTOR_ZERO;
	momentOfInertia_ = MOMENT_OF_INERTIA;			// 慣性モーメントの仮の値s
	dampingFactor_ = DAMPING_FACTOR;				// 減衰係数の仮の値
}

// 更新
void Stage::Update()
{
	// 物理的な傾きの更新
	// 角速度に基づいて角度を更新
	angle_.x += angularVelocity_.x;
	angle_.y += angularVelocity_.y;
	angle_.z += angularVelocity_.z;
	// 減衰を適用して角速度を減少させる
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


	// デバッグ表示
	//DrawFormatString(0, 400, GetColor(130, 255, 130), "Stage Angle: (%.2f, %.2f, %.2f)", AsoUtility::Rad2DegF(angle_.x), AsoUtility::Rad2DegF(angle_.y), AsoUtility::Rad2DegF(angle_.z));
	//DrawSphere3D(collider_.center, collider_.radius, 16, GetColor(255, 0, 0), GetColor(255, 0, 0), FALSE);
}

// 解放
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

// プレイヤーの位置に応じてステージを傾ける
void Stage::UpdateTilt(const std::vector<Player*>& players)
{
	// プレイヤーの機能実装まで傾けない
	return;

	// デバッグ用で特定のキーを押下したら傾けない
	InputManager& ins = InputManager::GetInstance();

	// F1キーで傾き無効化,もう一度押すと有効化
	if (ins.IsNew(KEY_INPUT_F1))
	{
		return;
	}


	// プレイヤーの情報がなければ傾けない
	if (players.empty()) return;

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

	// プレイヤーの平均位置を計算
	float totalWeight = 0.0f;
	float weightedX = 0.0f;
	float weightedZ = 0.0f;

	for (auto p : players)
	{
		// 生存中かつ、落下状態ではないプレイヤーのみを加算する
		if(!(p -> IsAlive() && !p ->IsFalling() && IsPlayerOnStage(p->GetPos())))
			// 処理をスキップ
			continue;

		float weight = p->GetWeight();
		VECTOR pos = p->GetPos();
		weightedX += (pos.x - this->pos_.x) * weight;
		weightedZ += (pos.z - this->pos_.z) * weight;
		totalWeight += weight;
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

VECTOR Stage::GetStageNormal() const
{
	// ステージの傾きを基に法線ベクトルを計算
	VECTOR up = { 0.0f, 1.0f, 0.0f };
	VECTOR v = VTransform(up, MGetRotX(angle_.x));
	v = VTransform(v, MGetRotZ(angle_.z));
	v = VTransform(v, MGetRotY(angle_.y));
	return v;

}
