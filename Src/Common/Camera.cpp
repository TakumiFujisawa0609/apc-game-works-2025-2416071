#include "Camera.h"

Camera::Camera() {
    pos_ = VGet(100.0f, 100.0f, -150.0f);  // 初期カメラ位置
    target_ = VGet(0.0f, 0.0f, 0.0f);      // 注視点
}

Camera::~Camera() {}

void Camera::Init() {
    // 初期化時に特に処理なし（コンストラクタで初期化済み）
}

void Camera::SetPosition(const VECTOR& pos) { pos_ = pos; }
void Camera::SetTarget(const VECTOR& target) { target_ = target; }

VECTOR Camera::GetPosition() const { return pos_; }
VECTOR Camera::GetTarget() const { return target_; }

void Camera::Update() {
    //デバッグ
  	if (CheckHitKey(KEY_INPUT_UP))    pos_.z += 0.5f;
  	if (CheckHitKey(KEY_INPUT_DOWN))  pos_.z -= 0.5f;
  	if (CheckHitKey(KEY_INPUT_LEFT))  pos_.x -= 0.5f;
  	if (CheckHitKey(KEY_INPUT_RIGHT)) pos_.x += 0.5f;
}

void Camera::Begin3D() {
    SetCameraPositionAndTarget_UpVecY(
        pos_,      // VECTOR 型
        target_   // VECTOR 型
    );
}

// 3D描画終了（2D描画モードに戻す）
void Camera::End3D() {
    SetDrawMode(DX_DRAWMODE_BILINEAR);
}
