#include "Tile.h"
#include <DxLib.h>

void Tile::Init(TYPE type, int hp, const VECTOR& pos, int baseModelId) {
    type_ = type;
    hp_ = hp;
    pos_ = pos;
    isHole_ = false;
    lastStepTime_ = 0;
    if (modelId_ != -1) MV1DeleteModel(modelId_);
    modelId_ = (baseModelId >= 0) ? MV1DuplicateModel(baseModelId) : -1;
}

void Tile::Damage(int amount)
{
    if (isHole_) return;
    if (type_ == TYPE::SAFE) return;
    hp_ -= amount;
    if (hp_ <= 0)
    {
        isHole_ = true;
    }
}

void Tile::OnStep()
{
    if (isHole_) return;
    if (type_ == TYPE::SAFE) return;
    int nowMs = GetNowCount();
    if (lastStepTime_ == 0 || (nowMs - lastStepTime_) >= STEP_DAMEGE_INTERVAL)
    {
        lastStepTime_ = nowMs;
        Damage(1);
    }
}

void Tile::Draw(const VECTOR& stageAngle) const
{
    if (isHole_ || modelId_ < 0) return;
    MV1SetPosition(modelId_, pos_);
    MV1SetRotationXYZ(modelId_, stageAngle); // ‚±‚±‚ÅŒX‚«‚ð”½‰fI
    MV1DrawModel(modelId_);
}

void Tile::Release()
{
    if (modelId_ != -1)
    {
        MV1DeleteModel(modelId_);
        modelId_ = -1;
    }
}