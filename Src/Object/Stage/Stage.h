#pragma once
#include <DxLib.h>
#include "../../Utility/AsoUtility.h"
class Stage 
{
public:
	// ステージの初期位置
	static constexpr VECTOR DEFAULT_POS = AsoUtility::VECTOR_ZERO;
	// コンストラクタ
	Stage(void);
	// デストラクタ
	 ~Stage(void);
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

private:

	// ステージモデル(3D)
	int modelId_;

	// 大きさ、角度、向き
	VECTOR scale_, angle_, pos_;

};

