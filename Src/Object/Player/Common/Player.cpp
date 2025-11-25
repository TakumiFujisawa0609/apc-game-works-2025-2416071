#include "../Control/InputController.h"
#include "../../Bullet/BulletManager.h"
#include "../../Stage/Stage.h"
#include "../../../Utility/MatrixUtility.h"
#include "../../../Utility/AsoUtility.h"
#include "../../../Manager/InputManager.h"
#include "PlayerManager.h"
#include "Player.h"
#include <DxLib.h>

// 静的メンバ
int Player::nextDeathOrder_ = 1;

// コンストラクタ
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
	// 変数の初期化 (リセット可能な状態をInitで設定)
	
	pos_ = { 0.0f, 0.0f, 0.0f }; 
	moveVec_ = { 0.0f,0.0f,0.0f };
	param_.speed = 15.0f;
	angle_ = { 0.0f, AsoUtility::Deg2RadF(180.0f), 1.0f };	// 初期向きはZ+方向
	modelId_ = -1;											// Initでロードしない場合は-1で初期化

	inputVecNor_ = { 0.0f, 0.0f, 1.0f };					// 初期入力方向はZ+方向
}

void Player::Update()
{
	// 落下時死亡判定チェック
	if (pos_.y < -1000.0f)
	{
		Die();
		pos_.y == -1000.0f; // 一応位置リセット
	}

	// ステージのインスタンスを取得
	Stage& stage = Stage::GetInstance();

	// ステージ外にいるかどうか
	VECTOR stageCenter = stage.GetPos();			// ステージの中心位置
	float stageRadius = stage.GetCollider().radius; // ステージの半径

	// プレイヤーの座標からステージの中心までの距離を計算
	float dx = pos_.x - stageCenter.x;
	float dz = pos_.z - stageCenter.z;
	float distXZ = sqrtf(dx * dx + dz * dz);

	// ステージ外に出たら落下する
	if (!isFalling_ && distXZ > stageRadius)
	{
		isFalling_ = true;
		// 水平速度をリセット
		moveVec_.x = 0.0f;
	}

	// 落下中の処理
	if (isFalling_)
	{
		// 落下速度を加算
		moveVec_.y -= GRAVITY_ACCEL; 

		// 位置更新
		pos_ = VAdd(pos_, moveVec_);

		// ステージ内に戻ったら落下状態解除
		if (distXZ <= stageRadius)
		{
			// 落下状態解除
			isFalling_ = false;
			

			// Y速度をリセット
			moveVec_.y = 0.0f;
		}
		return; 

	}

	// 移動処理
	Move();

	// 重力
	moveVec_.y -= GRAVITY_ACCEL;		// 常に重力を加算する

	// 摩擦処理
	const float PLAYER_FRICTION = 0.85f;
	moveVec_ = VScale(moveVec_, PLAYER_FRICTION);

	// 最大速度制限
	float len = VSize(moveVec_);
	if (len > param_.maxSpeed)
	{
		moveVec_ = VScale(moveVec_, param_.maxSpeed / len);
	}

	if (attackCooldown_ > 0.0f)
	{
		attackCooldown_ -= 1.0f; // フレームごとに減少
		if (attackCooldown_ < 0.0f)
			attackCooldown_ = 0.0f;
	}

	

	// 位置更新
	pos_ = VAdd(pos_, moveVec_);

	// プレイヤーの球体コライダー情報
	VECTOR center = pos_;
	float radius = collisionRadius_;

	// ステージとの衝突判定
	MV1_COLL_RESULT_POLY_DIM result = MV1CollCheck_Sphere(stage.GetModelID(), -1, center, radius);

	// 衝突しているポリゴンが1つ以上ある場合
	if (result.HitNum > 0)
	{
		for (int i = 0; i < result.HitNum; ++i)
		{
			const MV1_COLL_RESULT_POLY& poly = result.Dim[i];

			// 衝突点から法線方向にプレイヤーの中心までのベクトル
			VECTOR vToCenter = VSub(center, poly.HitPosition);

			// 法線との内積からめり込み深さを求める
			float currentDist = VDot(vToCenter, poly.Normal);
			float depth = radius - currentDist;

			// めり込み深さが正（めり込んでいる）の場合
			if (depth > 0.0f)
			{
				// 法線方向にめり込み分だけ押し戻すベクトルを計算
				VECTOR push = VScale(poly.Normal, depth);

				// プレイヤーの位置を補正
				pos_ = VAdd(pos_, push);

				// 押し戻しによって中心位置が変わったので、次のループのためにcenterを更新
				center = pos_;

				// 地面とみなせる（法線のY成分が上向き）衝突の場合、Y速度をリセット
				if (poly.Normal.y > PUSHBACK_THRESHOLD_Y) // 地面判定のしきい値
				{
					moveVec_.y = 0.0f;
				}
				// 壁（法線が横向き）の場合、速度を反射させる
				else if (poly.Normal.y < 0.1f) // ほぼ横向きと判断
				{
					// 移動ベクトルから法線方向の成分を引いて、跳ね返りのような動きにする
					float speedOnNormal = VDot(moveVec_, poly.Normal);

					if (speedOnNormal < 0) { // 壁に向かっている場合のみ
						// 弾性衝突ではなく、壁に沿った動きをシミュレート
						VECTOR projection = VScale(poly.Normal, speedOnNormal);
						moveVec_ = VSub(moveVec_, projection);
					}
				}
			}
		}
	}

	MV1CollResultPolyDimTerminate(result);

	// 弾の更新処理
	BulletManager::GetInstance().Update();

	// 8.モデルに反映
	MV1SetPosition(modelId_, pos_);
}

void Player::Move()
{
	// ステージのインスタンスを取得
	Stage& stage = Stage::GetInstance();
	// 角度を取得
	VECTOR stageAngle = stage.GetAngle();

	// 回転行列
	MATRIX rotX = MGetRotX(stageAngle.x);
	MATRIX rotZ = MGetRotZ(stageAngle.z);
	MATRIX stageRotMat = MMult(rotZ, rotX);
	MATRIX invStageRotationMat = MTranspose(stageRotMat); // 逆行列は転置行列

	// 入力ベクトルの取得
	VECTOR worldInputVec = controller_->GetMoveInputVector();

	// 入力ベクトルを正規化　0除算対策
	float inputLen = VSize(worldInputVec);

	// 入力がある場合のみ inputVecNor_ を更新する。
	if (worldInputVec.x != 0.0f || worldInputVec.z != 0.0f)
	{
		inputVecNor_ = VNorm(worldInputVec); // 入力方向の正規化
	}

	// 上り坂の判定に使用するベクトル（XZ平面での傾き方向）
	// ステージ傾きを基に坂の方向ベクトルを算出
	VECTOR slopeDir = VGet(sinf(stageAngle.z), 0.0f, sinf(stageAngle.x));
	// slopeFactor: inputVecNor_と坂の向きの内積
	float slopeFactor = VDot(inputVecNor_, slopeDir);

	// 基本の速度と最大速度を初期化
	float moveSpeed = param_.speed;
	float maxSpeed = param_.maxSpeed;


	// 1. 入力による加速度計算
	VECTOR inputAcc = AsoUtility::VECTOR_ZERO;
	if (worldInputVec.x != 0.0f || worldInputVec.z != 0.0f)
	{
		// 移動方向をステージの傾き補正逆行列で変換
		VECTOR moveDir = VTransform(inputVecNor_, invStageRotationMat);
		// Y成分は無視
		moveDir.y = 0.0f;

		// 調節後のmovespeedを使って加速度を計算
		inputAcc = VScale(moveDir, moveSpeed * INPUT_ACCEL_FACTOR); // INPUT_ACCEL_FACTORは別途定義
	}

	// 2. 下り坂・重力による滑り (X軸/Z軸両方向の滑り)
	const float GRAVITY = 9.8f;
	const float SLIDE_FACTOR = 0.5f;

	// 速度に加速度を加算
	VECTOR worldGravity = VGet(0.0f, -GRAVITY * SLIDE_FACTOR, 0.0f); // 滑り係数を重力に適用
	VECTOR slideAcc = VTransform(worldGravity, invStageRotationMat);
	slideAcc.y = 0.0f; // Y成分は無視

	// 移動ベクトル更新 (Updateの摩擦/速度制限前にMoveの加速度を加算)
	moveVec_ = VAdd(moveVec_, inputAcc);
	moveVec_ = VAdd(moveVec_, slideAcc);

	// 3. ジャンプ処理
	if (controller_->IsJumpTrigger())
	{
		// 地面にいる場合のみジャンプ可能
		if (moveVec_.y <= 0.1f) // 浮いていないか、または着地直後
		{
			moveVec_.y = param_.jumpPower;
		}
	}

}

void Player::Shot()
{
	// プレイヤーの向きを基に回転行列を作成
	float rotY = atan2f(-inputVecNor_.x, -inputVecNor_.z);
	MATRIX rotMat = MGetRotY(rotY);

	// 発射方向（プレイヤーの前方向）
	VECTOR shootDir = VGet(0.0f, 0.0f, -1.0f);
	shootDir = VTransform(shootDir, rotMat); // 向きに合わせて変換

	// 発射位置 プレイヤーの胴体から少し前方にオフセット
	VECTOR muzzleOffset = VGet(0.0f, 1.0f, -1.0f); // 胴体の高さと前方オフセット
	VECTOR muzzlePos = VAdd(pos_, VTransform(muzzleOffset, rotMat));

	// 弾速
	float bulletSpeed = 20.0f;

	// 弾を生成
	BulletManager::GetInstance().AddBullet(muzzlePos, shootDir, bulletSpeed, id_);
}

void Player::Draw()
{
	MV1SetPosition(modelId_, pos_);

	float rotY = atan2f(-inputVecNor_.x, -inputVecNor_.z); // XZ平面での角度を計算
	VECTOR rot = { 0.0f, rotY, 0.0f };

	MV1SetRotationXYZ(modelId_, rot);

	if (pos_.y >= -1000.0f)
		MV1DrawModel(modelId_);

	// 弾の描画処理
	BulletManager::GetInstance().Draw();

	// デバッグ表示
	DrawSphere3D(pos_, 0.5f, 16, GetColor(255, 0, 0), GetColor(255, 0, 0), TRUE);
	DrawLine3D(pos_, VAdd(pos_, moveVec_), GetColor(0, 255, 0));

	// 1Pには赤色、2Pには青色、3Pには緑色、4Pには黄色の球体を表示
	//DrawSphere3D(pos_, 50.0f, 16, GetColor(255 * (id_ == 0), 255 * (id_ == 2), 255 * (id_ == 1)), GetColor(255 * (id_ == 0), 255 * (id_ == 2), 255 * (id_ == 1)), TRUE);

	// プレイヤーの重さを表示
	char buffer[64];
	sprintf_s(buffer, "P%d Weight: %.2f", id_ + 1, param_.weight);
	DrawString(10, 30 + id_ * 20, buffer, GetColor(255, 255, 255));

	// プレイヤーの速度を表示
	sprintf_s(buffer, "P%d Speed: %.2f", id_ + 1, VSize(moveVec_));
	DrawString(200, 30 + id_ * 20, buffer, GetColor(255, 255, 255));

}

void Player::ApplyHit(const VECTOR& knockBack)
{
	// ノックバックを加算
	moveVec_ = VAdd(moveVec_, knockBack);

	// 少し浮かせる
	moveVec_.y = max(moveVec_.y, knockBack.y);
}

void Player::Die()
{
	// 既に死亡している場合は処理しない
	if (!isAlive_) return;

	// 死亡順序を設定し、次の順序に進める
	isAlive_ = false;

	// 死亡順序を設定
	deathOrder_ = nextDeathOrder_++;
}

void Player::Release()
{
	// 派生クラスで実装
}