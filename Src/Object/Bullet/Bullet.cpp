//#include "Bullet.h"
//#include "../Stage/Stage.h"
//#include "../../Utility/AsoUtility.h"
//
//Bullet::Bullet(const VECTOR& pos, const VECTOR& dir, float speed, int ownerId)
//	:pos_(pos), dir_(dir), speed_(speed), isAlive_(true), ownerId_(ownerId)
//{
//}
//
//Bullet::~Bullet()
//{
//}
//
//void Bullet::Update()
//{
//	prevPos_ = pos_; // 前フレーム位置を保存
//
//	// 位置更新
//	pos_ = VAdd(pos_, VScale(dir_, speed_));
//
//	// 画面外に出たら消滅
//	if (pos_.x < -BULLET_LIMIT || pos_.x > BULLET_LIMIT ||
//		pos_.y < -BULLET_LIMIT || pos_.y > BULLET_LIMIT ||
//		pos_.z < -BULLET_LIMIT || pos_.z > BULLET_LIMIT)
//	{
//		isAlive_ = false;
//	}
//
//	// ステージとの衝突判定
//	const Stage& stage = Stage::GetInstance();
//	const CylinderCollider& col = stage.GetCollider();
//
//	//  円柱衝突判定
//	VECTOR c = col.center;
//	VECTOR p0 = VSub(prevPos_, c);
//	VECTOR p1 = VSub(pos_, c);
//	VECTOR d = VSub(p1, p0);
//
//	float rad = col.radius;
//	float yMinRel = col.yMin - c.y;
//	float yMaxRel = col.yMax - c.y;
//
//	collidedWithStage_ = false;
//	float bestT = 1.0f;
//	VECTOR hitPosWorld = AsoUtility::VECTOR_ZERO;
//	VECTOR hitNor = AsoUtility::VECTOR_ZERO;
//
//	// 側面との交差
//	{
//		float dx = d.x;
//		float dz = d.z;
//		float px = p0.x;
//		float pz = p0.z;
//
//		float a = dx * dx + dz * dz;
//
//		if (a > 1e-8f)
//		{
//			float b = 2.0f * (px * dx + pz * dz);
//			float cterm = px * px + pz * pz - rad * rad;
//
//			float dist = b * b - 4.0f * a * cterm;
//			if (dist >= 0.0f)
//			{
//				float sqrtD = std::sqrt(dist);
//				float t1 = (-b - sqrtD) / (2.0f * a);
//				float t2 = (-b + sqrtD) / (2.0f * a);
//
//				// チェックする候補時刻を昇順に試す
//				float cand[2] = { t1, t2 };
//				for (int i = 0; i < 2; ++i)
//				{
//					float t = cand[i];
//					if (t < 0.0f || t > 1.0f) continue;
//					float yAt = p0.y + d.y * t;
//					if (yAt >= yMinRel && yAt <= yMaxRel)
//					{
//						if (t < bestT)
//						{
//							bestT = t;
//							VECTOR hitRel = { p0.x + d.x * t, yAt, p0.z + d.z * t };
//							hitPosWorld = VAdd(hitRel, c);
//
//							// 側面法線はXZ方向の外向きベクトル
//							VECTOR n = { hitRel.x, 0.0f, hitRel.z };
//							hitNor = VNorm(n);
//							collidedWithStage_ = true;
//						}
//					}
//				}
//			}
//		}
//	}
//
//	// 上下の当たり判定
//	if (std::abs(d.y) > 1e-8f)
//	{
//		// 下面
//		{
//			float t = (yMinRel - p0.y) / d.y;
//			if (t >= 0.0f && t <= 1.0f && t < bestT)
//			{
//				float ix = p0.x + d.x * t;
//				float iz = p0.z + d.z * t;
//				if (ix * ix + iz * iz <= rad * rad)
//				{
//					bestT = t;
//					VECTOR hitRel = { ix, yMinRel, iz };
//					hitPosWorld = VAdd(hitRel, c);
//					hitNor = { 0.0f, -1.0f, 0.0f };		// 下蓋の法線は下向き（yMinがステージ底なら）
//					collidedWithStage_ = true;
//				}
//			}
//		}
//
//		// 上面
//		{
//			float t = (yMaxRel - p0.y) / d.y;
//			if (t >= 0.0f && t <= 1.0f && t < bestT)
//			{
//				float ix = p0.x + d.x * t;
//				float iz = p0.z + d.z * t;
//				if (ix * ix + iz * iz <= rad * rad + 1e-4f)
//				{
//					bestT = t;
//					VECTOR hitRel = { ix, yMaxRel, iz };
//					hitPosWorld = VAdd(hitRel, c);
//					hitNor = { 0.0f, 1.0f, 0.0f }; // 上蓋の法線は上向き
//					collidedWithStage_ = true;
//				}
//			}
//		}
//	}
//
//	// 当たり判定処理
//	if (collidedWithStage_)
//	{
//		// 衝突点での速度方向の反射
//		VECTOR dirNorm = VNorm(dir_);
//		float dot = VDot(dirNorm, hitNor);
//		VECTOR reflect = VSub(dirNorm, VScale(hitNor, 2.0f * dot));
//		dir_ = VNorm(reflect); // 反射方向を更新（正規化）
//
//		// 衝突位置に押し出す（少し余裕をもたせる）
//		pos_ = VAdd(hitPosWorld, VScale(hitNor, 10.0f));
//
//		// prevPos_ は衝突前の位置としてそのままにしておく
//		return;
//	}
//
//	// 既存の単純な平面（旧実装の stageY）での反射（保険）
//	{
//		VECTOR stageNor = stage.GetStageNormal();
//		float stagePosY = stage.GetPos().y;
//
//		if ((prevPos_.y > stagePosY && pos_.y <= stagePosY) ||
//			(prevPos_.y < stagePosY && pos_.y >= stagePosY))
//		{
//			float denom = (pos_.y - prevPos_.y);
//			if (std::abs(denom) > 1e-6f) {
//				float t = (stagePosY - prevPos_.y) / denom;
//				VECTOR hitPos = {
//					prevPos_.x + (pos_.x - prevPos_.x) * t,
//					stagePosY,
//					prevPos_.z + (pos_.z - prevPos_.z) * t
//				};
//
//				float dot = VDot(dir_, stageNor);
//				VECTOR reflect = VSub(dir_, VScale(stageNor, 2.0f * dot));
//				dir_ = reflect;
//
//				pos_ = VAdd(hitPos, VScale(stageNor, 10.0f));
//				return;
//			}
//		}
//	}
//}
//
//void Bullet::Draw() const
//{
//	DrawSphere3D(pos_, 30.0f, 16, GetColor(255, 0, 0), GetColor(255, 0, 0), TRUE);
//
//	// デバッグ表示
//	// 生きている弾の座標を表示
//	//DrawFormatString(0, 200, GetColor(255, 255, 255), "Bullet Pos: (%.2f, %.2f, %.2f)", pos_.x, pos_.y, pos_.z);
//
//	// 弾が移動した線を表示
//	//DrawLine3D(prevPos_, pos_, GetColor(0, 255, 0));
//
//	// 弾がステージと当たったかどうかを表示
//	//DrawFormatString(0, 220, GetColor(255, 255, 255), "Bullet Collided: %s", collidedWithStage_ ? "Yes" : "No");
//}
//
//void Bullet::Release()
//{
//	// 特になし
//}
