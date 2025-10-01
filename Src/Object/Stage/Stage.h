#pragma once
#include <DxLib.h>
#include "../../Utility/AsoUtility.h"

class Player;

struct CylinderCollider
{
	VECTOR center; // XZ座標の中心
	float radius;  // 円の半径
	float yMin;    // 床
	float yMax;    // 天井
};


class Stage 
{
public:
	
	// ステージの初期座標
	static constexpr VECTOR DEFAULT_POS = { 0.0f, -750.0f, 0.0f };

	 // 明示的なインスタンスを生成
	static void CreateInstance(void);
	// インスタンスを取得
	static Stage& GetInstance(void);

	// 初期化処理
	 void Init(void) ;
	// 更新ステップ
	 void Update(void) ;
	// 描画処理
	 void Draw(void);

	// 解放処理
	 void Release(void) ;

	// パラメータ
	void SetParam(int playerNum);

	// アングル
	void SetAngle(const VECTOR& angle) { angle_ = angle; }

	VECTOR GetAngle() const { return angle_; }

	// ステージの位置取得
	VECTOR GetPos() const { return pos_; }

	// ステージの当たり判定
	bool IsInsideStage(const VECTOR& pos) const;

	// コライダー取得
	const CylinderCollider& GetCollider() const { return cylinder_; }

private:

	// コンストラクタ
	Stage(void);
	// デストラクタ
	~Stage(void);

	// ステージモデル(3D)
	int modelId_;

	// 大きさ、角度、向き
	VECTOR scale_, angle_, pos_;

	// インスタンス
	static Stage* instance_;

	// プレイヤーのインスタンスを取得
	Player* player_;

	// コライダー情報
	CylinderCollider cylinder_;

};

