#pragma	once
#include <DxLib.h>
#include <string>
#include "../Stage/Stage.h"
#include "../../Manager/InputManager.h"

class Player
{
public:

	// コンストラクタ
	// ID、重さを設定
	Player(int id , float weight) { ; }

	// デストラクタ
	virtual ~Player() = default;

	// 初期化処理
	// 後からすべて純粋仮想関数に変更

	virtual void Init(void) { ; }

	// 更新
	virtual void Update(void) {};

	// 描画
	virtual void Draw(void) {};

	// 攻撃
	virtual void Attack(void) {};

	// 解放処理は派生クラスで各自開放

	// プレイヤーの識別IDを取得
	int GetID(void) const { return id_; }

	// プレイヤーのcharacter名識別
	virtual std::string GetCharacterName(void) const { return characterName_; }

	// プレイヤーの位置取得
	VECTOR GetPos(void) const { return pos_; }
	// プレイヤーの位置設定
	void SetPos(const VECTOR& pos) { pos_ = pos; }

	// プレイヤーの半径取得
	float GetRadius(void) const { return radius_; }

	// プレイヤーの高さ取得
	float GetHeight(void) const { return height_; }

	// 移動処理（基底クラスで実装）
	void UpdateMove(void);

	// 重さを取得
	float GetWeight(void) const { return weight_; }

protected:

	// 変数はすべてここで初期化

	// 識別ID
	int id_ = -1;

	// キャラクター名識別
	std::string characterName_ = "NoName";

	// モデルID
	int modelId_ = -1;

	// 位置
	VECTOR pos_ = VGet(0.0f, 0.0f, 0.0f);

	// 半径
	float radius_ = 1.0f;

	// 高さ
	float height_ = 2.0f;

	// 移動速度
	float speed_ = 10.2f;

	// ジャンプ力
	float jumpPower_ = 10.5f;

	// 重力
	float gravity_ = -0.5f;

	// Y軸速度
	float velY_ = 0.0f;

	// 地面に接地しているか
	bool isOnGround_ = false;

	// それぞれのプレイヤーの重さ
	float weight_ = 1.0f;

	// 向き
	VECTOR angle_ = VGet(0.0f, 0.0f, 0.0f);
};