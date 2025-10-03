#include "Stage.h"
#include <DxLib.h>
#include "../Player/Player.h"
#include "../Player/PlayerManager.h"

Stage* Stage::instance_ = nullptr;




Stage::Stage(void)
{
}

Stage::~Stage(void)
{
}

void Stage::CreateInstance(void)
{
    if (instance_ == nullptr)
    {
        instance_ = new Stage();
    }
    instance_->Init();
}

Stage& Stage::GetInstance(void)
{
    if (instance_ == nullptr)
    {
        Stage::CreateInstance();
    }
    return *instance_;
}

void Stage::Init(void)
{
    // モデルの読み込み
    modelId_ = MV1LoadModel("Data/Model/Stage/Stage.mv1");

    if (modelId_ == -1)
    {
        DxLib::DxLib_End();
        exit(-1);
    }

    pos_ = DEFAULT_POS;
    angle_ = { 0.0f, 0.0f, 0.0f };
    scale_ = { 1.0f, 1.0f, 1.0f };

    // 円柱コライダー設定（仮の半径/高さ）
    cylinder_.center = pos_;
    cylinder_.radius = 300.0f;
    cylinder_.yMin = pos_.y;            // 床
    cylinder_.yMax = pos_.y + 1000.0f;  // 高さ100
}

void Stage::Update()
{
    // PlayerManager から全プレイヤー取得
    auto players = PlayerManager::GetInstance().GetPlayerRawPlayers(); // 生ポインタ配列

    // プレイヤーの重さに応じて傾きを更新
    UpdateTilt(players);
}


void Stage::Draw(void)
{
	// モデルの描画
	MV1SetPosition(modelId_, pos_);
	MV1SetRotationXYZ(modelId_, angle_);
	MV1SetScale(modelId_, scale_);
	MV1DrawModel(modelId_);
}

void Stage::Release(void)
{
	MV1DeleteModel(modelId_);
}

void Stage::SetParam(int playerNum)
{

}

bool Stage::IsInsideStage(const VECTOR& pos) const
{

    // 仮でx,zが±500以内ならステージ内とする
    if (pos.x < 500.0f && pos.x > -500.0f &&
        pos.z < 500.0f && pos.z > -500.0f)
    {
        return true;
    }
}


float Stage::RayGroundHeight(const VECTOR& pos)
{
    MV1_COLL_RESULT_POLY result{};

	// 初期化
	
    // プレイヤーの頭上からレイを飛ばす
    VECTOR from = VGet(pos.x, pos.y + 1000.0f, pos.z);
	VECTOR to = VGet(pos.x, pos.y - 1000.0f, pos.z);

	// レイの当たり判定
    MV1CollCheck_Line(modelId_, -1, from, to);

    if (result.HitFlag)  // 当たったかどうか判定
    {
		return result.HitPosition.y; // 当たった位置のY座標を返す
    }

	return 0.0f; // 当たらなかった場合は-1を返す
}

void Stage::UpdateTilt(const std::vector<Player*>& players)
{
    float totalWeight = 0.0f;
    float weightedX = 0.0f;
    float weightedZ = 0.0f;

    for (auto p : players)
    {
        float w = p->GetWeight();
        weightedX += (p->GetPos().x - pos_.x) * w;
        weightedZ += (p->GetPos().z - pos_.z) * w;
        totalWeight += w;
    }

    if (totalWeight == 0.0f) return;

    float centerX = weightedX / totalWeight;
    float centerZ = weightedZ / totalWeight;

    // =========================
    // プレイヤーがいる側に傾くように符号調整
    // =========================
    float tiltFactor = 0.0005f; // 調整用 (小さめにすると自然)

    angle_.x = centerZ * tiltFactor;  // Z方向の偏りでX軸回転
    angle_.z = -centerX * tiltFactor;  // X方向の偏りでZ軸回転

    MV1SetRotationXYZ(modelId_, angle_);
}

float Stage::GetGroundHeight(const VECTOR& pos)
{
    // ステージ中心を原点としたローカルXZ
    float dx = pos.x - pos_.x;
    float dz = pos.z - pos_.z;

    // 傾きを使って高さ計算
    float groundY = pos_.y; // 基本高さ
    groundY = pos_.y
        + sinf(angle_.x) * dz   // X軸回転による傾き補正
        + sinf(angle_.z) * dx;  // Z軸回転による傾き補正

    return groundY;
}





