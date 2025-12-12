#include "../Control/InputController.h"
#include "../../Bullet/BulletManager.h"
#include "../../Stage/Stage.h"
#include "../../../Utility/MatrixUtility.h"
#include "../../../Utility/AsoUtility.h"
#include "../../../Manager/InputManager.h"
#include "PlayerManager.h"
#include "Player.h"
#include <DxLib.h>

int Player::nextDeathOrder_ = 1;

Player::Player(int id, const PlayerParam& param, std::unique_ptr<InputController> controller)
	: id_(id),
	param_(param),
	controller_(std::move(controller))
{
}

Player::~Player()
{
}

void Player::Init()
{
	pos_ = { 0.0f, 0.0f, 0.0f };
	moveVec_ = { 0.0f,0.0f,0.0f };
	param_.speed = 15.0f;
	angle_ = { 0.0f, AsoUtility::Deg2RadF(180.0f), 1.0f };
	modelId_ = -1;

	inputVecNor_ = { 0.0f, 0.0f, 1.0f };

	// 弾数初期化
	ammoCount_ = AMMO_MAX;
	ammoRegenTimerSec_ = 0.0f;

	// 反動初期値（必要ならここで個別調整可能）
	recoilForce_ = DEFAULT_RECOIL_FORCE;
	recoilVerticalBonus_ = DEFAULT_RECOIL_VERTICAL_BONUS;
}

void Player::Update()
{
	Stage& stage = Stage::GetInstance();

	MATRIX rotX = MGetRotX(stage.GetAngle().x);
	MATRIX rotZ = MGetRotZ(stage.GetAngle().z);
	MATRIX stageRotMat = MMult(rotZ, rotX);
	MATRIX invStageRotMat = MTranspose(stageRotMat);

	VECTOR stageCenter = stage.GetPos();
	float stageRadius = stage.GetCollider().radius;

	VECTOR rel = VSub(pos_, stageCenter);
	VECTOR local = VTransform(rel, invStageRotMat);
	float distLocalXZ = sqrtf(local.x * local.x + local.z * local.z);
	bool insideLocalRadius = (distLocalXZ <= stageRadius);

	if (!isFalling_)
	{
		Move();
		moveVec_.y -= GRAVITY_ACCEL;
		moveVec_ = VScale(moveVec_, PLAYER_FRICTION);

		float len = VSize(moveVec_);
		if (len > param_.maxSpeed)
		{
			moveVec_ = VScale(moveVec_, param_.maxSpeed / len);
		}
	}
	else
	{
		moveVec_.y -= GRAVITY_ACCEL;
	}

	// 位置更新
	pos_ = VAdd(pos_, moveVec_);

	// 衝突解決（既存）
	const int MAX_SOLVE = 3;
	bool grounded = false;
	for (int iter = 0; iter < MAX_SOLVE; ++iter)
	{
		MV1_COLL_RESULT_POLY_DIM result =
			MV1CollCheck_Sphere(stage.GetModelID(), -1, pos_, collisionRadius_);

		if (result.HitNum == 0) break;

		bool anyAdjust = false;
		for (int i = 0; i < result.HitNum; ++i)
		{
			const auto& poly = result.Dim[i];
			VECTOR toCenter = VSub(pos_, poly.HitPosition);
			float penetration = VDot(toCenter, poly.Normal);
			float depth = collisionRadius_ - penetration;

			if (depth > 0.0f)
			{
				VECTOR push = VScale(poly.Normal, depth);
				pos_ = VAdd(pos_, push);
				anyAdjust = true;

				if (poly.Normal.y > PUSHBACK_THRESHOLD_Y)
				{
					grounded = true;
					if (moveVec_.y < 0.0f) moveVec_.y = 0.0f;
				}
				else if (poly.Normal.y < 0.1f)
				{
					float speedOnNormal = VDot(moveVec_, poly.Normal);
					if (speedOnNormal < 0.0f)
					{
						VECTOR proj = VScale(poly.Normal, speedOnNormal);
						moveVec_ = VSub(moveVec_, proj);
					}
				}
			}
		}

		if (!anyAdjust) break;
	}

	if (!grounded && !insideLocalRadius)
	{
		isFalling_ = true;
	}
	else if (grounded)
	{
		isFalling_ = false;
	}

	if (pos_.y < -1000.0f)
	{
		Die();
	}

	// 弾の時間回復（ゲーム開始30秒後、勝敗未決のみ）
	{
		PlayerManager& pm = PlayerManager::GetInstance();
		if (!pm.GetIsGameOver())
		{
			int nowMs = GetNowCount();
			int startMs = pm.GetGameStartTimeMs();
			if (nowMs - startMs >= 30000)
			{
				ammoRegenTimerSec_ += 1.0f / 60.0f;
				if (ammoRegenTimerSec_ >= AMMO_REGEN_INTERVAL_SEC)
				{
					ammoRegenTimerSec_ = 0.0f;
					if (ammoCount_ < AMMO_MAX) ammoCount_++;
				}
			}
		}
	}

	// 弾の更新
	BulletManager::GetInstance().Update();

	// モデル更新
	if (modelId_ != -1)
	{
		MV1SetPosition(modelId_, pos_);
		float rotY = atan2f(-inputVecNor_.x, -inputVecNor_.z);
		MV1SetRotationXYZ(modelId_, VGet(0.0f, rotY, 0.0f));
	}
}

void Player::Move()
{
	Stage& stage = Stage::GetInstance();
	VECTOR stageAngle = stage.GetAngle();

	MATRIX rotX = MGetRotX(stageAngle.x);
	MATRIX rotZ = MGetRotZ(stageAngle.z);
	MATRIX stageRotMat = MMult(rotZ, rotX);
	MATRIX invStageRotationMat = MTranspose(stageRotMat);

	VECTOR worldInputVec = controller_->GetMoveInputVector();

	float inputLen = VSize(worldInputVec);

	if (worldInputVec.x != 0.0f || worldInputVec.z != 0.0f)
	{
		inputVecNor_ = VNorm(worldInputVec);
	}

	VECTOR slopeDir = VGet(sinf(stageAngle.z), 0.0f, sinf(stageAngle.x));
	float slopeFactor = VDot(inputVecNor_, slopeDir);

	float moveSpeed = param_.speed;
	float maxSpeed = param_.maxSpeed;

	VECTOR inputAcc = AsoUtility::VECTOR_ZERO;
	if (worldInputVec.x != 0.0f || worldInputVec.z != 0.0f)
	{
		VECTOR moveDir = VTransform(inputVecNor_, invStageRotationMat);
		moveDir.y = 0.0f;

		inputAcc = VScale(moveDir, moveSpeed * INPUT_ACCEL_FACTOR);
	}

	const float GRAVITY = 9.8f;
	const float SLIDE_FACTOR = 0.5f;

	VECTOR worldGravity = VGet(0.0f, -GRAVITY * SLIDE_FACTOR, 0.0f);
	VECTOR slideAcc = VTransform(worldGravity, invStageRotationMat);
	slideAcc.y = 0.0f;

	moveVec_ = VAdd(moveVec_, inputAcc);
	moveVec_ = VAdd(moveVec_, slideAcc);

	if (controller_->IsJumpTrigger())
	{
		if (moveVec_.y <= 0.1f)
		{
			moveVec_.y = param_.jumpPower;
		}
	}
}

void Player::Shot()
{
	// 弾数チェック
	if (ammoCount_ <= 0) return;

	// 消費
	ammoCount_--;

	// プレイヤーの向きに合わせて弾発射
	float rotY = atan2f(-inputVecNor_.x, -inputVecNor_.z);
	MATRIX rotMat = MGetRotY(rotY);

	VECTOR shootDir = VGet(0.0f, 0.0f, -1.0f);
	shootDir = VTransform(shootDir, rotMat);

	VECTOR muzzleOffset = VGet(0.0f, 1.0f, -1.0f);
	VECTOR muzzlePos = VAdd(pos_, VTransform(muzzleOffset, rotMat));

	float bulletSpeed = 20.0f;

	BulletManager::GetInstance().AddBullet(muzzlePos, shootDir, bulletSpeed, id_);

	// 発射側の反動（進行方向逆）
	VECTOR recoil = shootDir;
	recoil.y = 0.0f;
	float len = VSize(recoil);
	if (len > 1e-5f) recoil = VScale(recoil, 1.0f / len);
	// 水平に反動を付与
	moveVec_ = VAdd(moveVec_, VScale(recoil, -recoilForce_));
	// 空中時にわずかな上方向反動を追加したい場合（今回は0.0fなので実質無し）
	if (recoilVerticalBonus_ > 0.0f && isFalling_)
	{
		moveVec_.y += recoilVerticalBonus_;
	}
}

void Player::Draw()
{
	MV1SetPosition(modelId_, pos_);

	float rotY = atan2f(-inputVecNor_.x, -inputVecNor_.z);
	VECTOR rot = { 0.0f, rotY, 0.0f };

	MV1SetRotationXYZ(modelId_, rot);

	if (pos_.y >= -1000.0f)
		MV1DrawModel(modelId_);

	BulletManager::GetInstance().Draw();

	DrawSphere3D(pos_, 0.5f, 16, GetColor(255, 0, 0), GetColor(255, 0, 0), TRUE);
	DrawLine3D(pos_, VAdd(pos_, moveVec_), GetColor(0, 255, 0));

	char buffer[64];
	sprintf_s(buffer, "P%d Weight: %.2f", id_ + 1, param_.weight);
	DrawString(10, 30 + id_ * 20, buffer, GetColor(255, 255, 255));

	sprintf_s(buffer, "P%d Speed: %.2f", id_ + 1, VSize(moveVec_));
	DrawString(200, 30 + id_ * 20, buffer, GetColor(255, 255, 255));
}

void Player::ApplyHit(const VECTOR& knockBack)
{
	moveVec_ = VAdd(moveVec_, knockBack);
	moveVec_.y = max(moveVec_.y, knockBack.y);
}

void Player::Die()
{
	if (!isAlive_) return;
	isAlive_ = false;
	deathOrder_ = nextDeathOrder_++;
}

void Player::Release()
{
	// ドメイン依存の後始末
}