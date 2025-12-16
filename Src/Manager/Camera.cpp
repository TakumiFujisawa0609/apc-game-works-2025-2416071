#include "../Utility/AsoUtility.h"
#include "../Manager/InputManager.h"
#include "../Object/Player/Common/PlayerManager.h"
#include "../Object/Stage/Stage.h"
#include "Camera.h"
#include <cmath>

Camera::Camera(void)
{
	// DxLib デフォルトはコメントの通り
}

Camera::~Camera(void)
{
}

void Camera::Init(void)
{
	// 初期位置と角度
	pos_ = DEFAULT_POS;
	angles_ = DEFAULT_ANGLES;

	// 初期は固定ポイント（＝本作では追従カメラとして利用）
	mode_ = MODE::FIXED_POINT;
}

void Camera::Update(void)
{
	// 追従カメラ（平均注視＋X軸回転のみ）
	// プレイヤーの平均座標を求める（生存者のみ）
	auto& pm = PlayerManager::GetInstance();
	const auto players = pm.GetPlayerRawPlayers();

	int alive = 0;
	VECTOR sum = { 0.0f, 0.0f, 0.0f };

	for (auto* p : players)
	{
		if (!p) continue;
		if (!p->IsAlive()) continue;
		VECTOR po = p->GetPos();
		sum.x += po.x; sum.y += po.y; sum.z += po.z;
		++alive;
	}

	VECTOR target;
	if (alive > 0)
	{
		target = { sum.x / alive, sum.y / alive, sum.z / alive };
	}
	else
	{
		// 誰もいなければステージ中心
		target = Stage::GetInstance().GetPos();
	}

	// プレイヤーの散開量（XZ平面）から距離をオートズーム
	float dist = baseDistance_;
	if (alive > 0)
	{
		float maxSpread = 0.0f;
		for (auto* p : players)
		{
			if (!p || !p->IsAlive()) continue;
			const VECTOR po = p->GetPos();
			const float dx = po.x - target.x;
			const float dz = po.z - target.z;
			const float spread = std::sqrt(dx * dx + dz * dz);
			if (spread > maxSpread) maxSpread = spread;
		}
		dist = baseDistance_ + maxSpread * zoomSpreadScale_;
		if (dist < minDistance_) dist = minDistance_;
		if (dist > maxDistance_) dist = maxDistance_;
	}

	// X軸回転（ピッチ）のみでカメラ位置を決定
	// forward（視線方向）は (0, -sinθ, cosθ) を想定
	// target = cameraPos + forward * dist となるように cameraPos を計算
	const float pitch = angles_.x; // ラジアン（DEFAULT_ANGLES 初期値を使用）
	const float sinP = std::sinf(pitch);
	const float cosP = std::cosf(pitch);

	const VECTOR desiredPos = {
		target.x,
		target.y + sinP * dist,
		target.z - cosP * dist
	};

	// スムージングして追従
	pos_ = LerpV(pos_, desiredPos, lerpFactor_);

	// 固定カメラでは Yaw/Roll は使わない（X軸回転のみ）
	angles_.y = 0.0f;
	angles_.z = 0.0f;
}

void Camera::SetBeforeDraw(void)
{
	// クリップ距離を設定
	SetCameraNearFar(VIEW_NEAR, VIEW_FAR);

	switch (mode_)
	{
	case MODE::FIXED_POINT:
		SetBeforeDrawFixedPoint();
		break;

	case MODE::FREE:
		// 自由移動・回転
		MoveXYZDirection();
		// SetBeforeDrawFree(); // 必要なら切り替え
		break;

	case MODE::NONE:
	default:
		break;
	}

	// カメラの設定（位置＋オイラー角）
	SetCameraPositionAndAngle(
		pos_,
		angles_.x,
		angles_.y,
		angles_.z
	);
}

void Camera::SetBeforeDrawFixedPoint(void)
{
	// 位置・角度は Update() 側で更新済み
}

void Camera::MoveXYZDirection(void)
{
	auto& ins = InputManager::GetInstance();

	// 矢印キーで角度変更
	float rotPow = 1.0f * DX_PI_F / 180.0f;
	if (ins.IsNew(KEY_INPUT_DOWN)) { angles_.x += rotPow; }
	if (ins.IsNew(KEY_INPUT_UP)) { angles_.x -= rotPow; }
	if (ins.IsNew(KEY_INPUT_RIGHT)) { angles_.y += rotPow; }
	if (ins.IsNew(KEY_INPUT_LEFT)) { angles_.y -= rotPow; }

	// IJKLで平行移動（デバッグ）
	const float movePow = 3.0f;
	VECTOR dir = AsoUtility::VECTOR_ZERO;
	if (ins.IsNew(KEY_INPUT_I)) { dir = { 0.0f, 0.0f, 1.0f }; }
	if (ins.IsNew(KEY_INPUT_J)) { dir = { -1.0f, 0.0f, 0.0f }; }
	if (ins.IsNew(KEY_INPUT_K)) { dir = { 0.0f, 0.0f, -1.0f }; }
	if (ins.IsNew(KEY_INPUT_L)) { dir = { 1.0f, 0.0f, 0.0f }; }
	if (!AsoUtility::EqualsVZero(dir))
	{
		MATRIX mat = MGetIdent();
		mat = MMult(mat, MGetRotX(angles_.x));
		mat = MMult(mat, MGetRotY(angles_.y));
		VECTOR moveDir = VTransform(dir, mat);
		pos_ = VAdd(pos_, VScale(moveDir, movePow));
	}
}

void Camera::SetBeforeDrawFree(void)
{
	// デバッグ自由カメラ（未使用）
	auto& input = InputManager::GetInstance();

	float rotPow = 1.f * DX_PI_F / 180.f;
	if (input.IsNew(KEY_INPUT_DOWN)) { angles_.x += rotPow; }
	if (input.IsNew(KEY_INPUT_UP)) { angles_.x -= rotPow; }
	if (input.IsNew(KEY_INPUT_LEFT)) { angles_.y += rotPow; }
	if (input.IsNew(KEY_INPUT_RIGHT)) { angles_.y -= rotPow; }

	float movePow = 3.f;
	if (input.IsNew(KEY_INPUT_W)) { pos_.z += movePow; }
	if (input.IsNew(KEY_INPUT_A)) { pos_.x -= movePow; }
	if (input.IsNew(KEY_INPUT_S)) { pos_.z -= movePow; }
	if (input.IsNew(KEY_INPUT_D)) { pos_.x += movePow; }
	if (input.IsNew(KEY_INPUT_Q)) { pos_.y += movePow; }
	if (input.IsNew(KEY_INPUT_E)) { pos_.y -= movePow; }
}

void Camera::DrawDebug(void)
{
	DrawFormatString(0, 600, GetColor(255, 0, 0),
		"Camera Pos:(%.1f, %.1f, %.1f)", pos_.x, pos_.y, pos_.z);
	//DrawFormatString(0, 620, GetColor(255, 255, 255),
	//	"Camera Angles:(%.2f, %.2f, %.2f)", AsoUtility::Rad2DegF(angles_.x), AsoUtility::Rad2DegF(angles_.y), AsoUtility::Rad2DegF(angles_.z));
}

void Camera::ChangeMode(MODE mode)
{
	mode_ = mode;

	switch (mode_)
	{
	case Camera::MODE::FIXED_POINT:
		// 追従は Update() が担当
		break;
	case Camera::MODE::FREE:
		// デバッグ用
		break;
	case Camera::MODE::NONE:
	default:
		break;
	}
}

void Camera::Release(void)
{
}

const VECTOR& Camera::GetPos(void) const
{
	return pos_;
}

const VECTOR& Camera::GetAngles(void) const
{
	return angles_;
}