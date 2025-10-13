#include "KeyController.h"
#include "../../../Utility/AsoUtility.h"

KeyController::KeyController(const KeyConfig& config)
	: config_(config)
{
}

VECTOR KeyController::GetMoveInputVector() const
{
	InputManager& ins = InputManager::GetInstance();
	VECTOR worldInputVec = { 0.0f, 0.0f, 0.0f };

	if (ins.IsNew(config_.up)) worldInputVec.z += 10.0f;
	if (ins.IsNew(config_.down)) worldInputVec.z -= 10.0f;
	if (ins.IsNew(config_.left)) worldInputVec.x -= 10.0f;
	if (ins.IsNew(config_.right)) worldInputVec.x += 10.0f;

	return worldInputVec;
}

bool KeyController::IsJumpTrigger() const
{
	InputManager& ins = InputManager::GetInstance();
	return ins.IsTrgDown(config_.jump);
}
