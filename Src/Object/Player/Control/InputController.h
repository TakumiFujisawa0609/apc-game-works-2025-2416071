#pragma once
#include <DxLib.h> // VECTOR型のために必要

// プレイヤーが使用するキーセットの定義をグローバルスコープに配置
struct KeyConfig
{
	int up;		// 前進
	int down;	// 後退
	int left;	// 左移動
	int right;	// 右移動
	int jump;	// ジャンプ
};

class InputController
{
public:
	// 仮想デストラクタ
	virtual ~InputController() = default;

	// 入力ベクトルを取得する純粋仮想関数を定義
	virtual VECTOR GetMoveInputVector() const = 0;

	// ジャンプトリガーを取得する純粋仮想関数を定義
	virtual bool IsJumpTrigger() const = 0;
};


