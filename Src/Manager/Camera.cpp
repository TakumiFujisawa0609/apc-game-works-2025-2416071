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

	// 初期は固定ポイント（＝追従カメラとして利用）
	mode_ = MODE::FIXED_POINT;

	// 追従設定の初期値
	followPlayers_ = false;
	autoZoom_ = true;
	baseDistance_ = 1400.0f;
	zoomSpreadScale_ = 1.5f;
	minDistance_ = 800.0f;
	maxDistance_ = 2600.0f;
	lerpFactor_ = 0.12f;
	zoomRampFrames_ = 30;
	zoomRampCounter_ = 0;
	hasLastTarget_ = false;
}

void Camera::ResetForGame(float pitchDeg, float baseDistance, bool enableAutoZoom, float zoomSpreadScale)
{
	// ピッチを設定（Yaw/Rollは0）
	angles_.x = pitchDeg * DX_PI_F / 180.0f;
	angles_.y = 0.0f;
	angles_.z = 0.0f;

	// 追従フラグとズーム設定
	followPlayers_ = true;
	autoZoom_ = enableAutoZoom;
	baseDistance_ = baseDistance;
	zoomSpreadScale_ = zoomSpreadScale;
	zoomRampCounter_ = 0;         // 最初は固定距離
	hasLastTarget_ = false;     // ターゲット未定義

	// 初期位置はステージ中心＋固定距離（人数に依らず一定）
	const VECTOR center = Stage::GetInstance().GetPos();
	const float sinP = std::sinf(angles_.x);
	const float cosP = std::cosf(angles_.x);
	pos_ = { center.x, center.y + sinP * baseDistance_, center.z - cosP * baseDistance_ };
	lastTarget_ = center;
	hasLastTarget_ = true;
}

void Camera::Update(void)
{
	// FIXED_POINT で追従有効時のみ処理
	if (mode_ != MODE::FIXED_POINT || !followPlayers_) return;

	// 生存プレイヤーの平均座標
	const auto players = PlayerManager::GetInstance().GetPlayerRawPlayers();
	int alive = 0;
	VECTOR sum = { 0.0f, 0.0f, 0.0f };
	for (auto* p : players)
	{
		if (!p || !p->IsAlive()) continue;
		const VECTOR po = p->GetPos();
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
		target = Stage::GetInstance().GetPos();
	}

	// オートズーム距離
	float dist = baseDistance_;
	if (autoZoom_)
	{
		if (zoomRampCounter_ >= zoomRampFrames_)
		{
			// 散開量（XZ）から距離を算出
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
		else
		{
			// ランプ中は固定距離
			++zoomRampCounter_;
			dist = baseDistance_;
		}
	}

	// X軸回転のみで視点位置を算出
	const float sinP = std::sinf(angles_.x);
	const float cosP = std::cosf(angles_.x);
	const VECTOR desiredPos = { target.x, target.y + sinP * dist, target.z - cosP * dist };

	// ターゲットと視点をスムージング
	if (!hasLastTarget_) { lastTarget_ = target; hasLastTarget_ = true; }
	lastTarget_ = LerpV(lastTarget_, target, lerpFactor_);
	pos_ = LerpV(pos_, desiredPos, lerpFactor_);

	// Yaw/Rollは使わない
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
	if (ins.IsNew(KEY_INPUT_I)) { dir = { 0.0f, 0.0f,  1.0f }; }
	if (ins.IsNew(KEY_INPUT_J)) { dir = { -1.0f, 0.0f,  0.0f }; }
	if (ins.IsNew(KEY_INPUT_K)) { dir = { 0.0f, 0.0f, -1.0f }; }
	if (ins.IsNew(KEY_INPUT_L)) { dir = { 1.0f, 0.0f,  0.0f }; }
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
	/*DrawFormatString(0, 600, GetColor(255, 0, 0),
		"Camera Pos:(%.1f, %.1f, %.1f)", pos_.x, pos_.y, pos_.z);*/
	//DrawFormatString(0, 620, GetColor(255, 255, 255),
	//	"Camera Angles:(%.2f, %.2f, %.2f)", AsoUtility::Rad2DegF(angles_.x), AsoUtility::Rad2DegF(angles_.y), AsoUtility::Rad2DegF(angles_.z));
}

void Camera::ChangeMode(MODE mode)
{
	mode_ = mode;
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