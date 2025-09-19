#pragma once
#include <DxLib.h>

class Camera
{
public:
    Camera();
    ~Camera();

    void Init();

    void SetPosition(const VECTOR& pos);
    void SetTarget(const VECTOR& target);

    //追記:ターゲットの追尾や初期位置等は各クラスで変更するようにしてください
    VECTOR GetPosition() const;
    VECTOR GetTarget() const;

    void Update();

    // 追加：3D描画開始・終了
    void Begin3D();
    void End3D();

private:
    VECTOR pos_;     // カメラの座標
    VECTOR target_;  // 注視点
};


