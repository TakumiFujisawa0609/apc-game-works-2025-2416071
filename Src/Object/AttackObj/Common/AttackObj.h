#pragma once
#include <DxLib.h>

// 前方クラス宣言
class Player;

// 攻撃オブジェクト基底クラス
class AttackObj
{
public:

	// コンストラクタ
	// デフォルトコンストラクタではないので、各自派生クラスで呼びだし
	AttackObj(int ownerId, const VECTOR& startPos, const VECTOR& direction, float speed);

	// デストラクタ
	virtual ~AttackObj() = default;

	// 基本処理
	// 純粋仮想関数で実装するので、派生クラスで必ず実装すること
	virtual void Init();
	virtual void Update() = 0;
	virtual void Draw() = 0;

	// 衝突時の処理(共通処理)
	virtual void OnCollision(Player& player) = 0;

protected:

	VECTOR pos_;         // 位置
	VECTOR moveVec_;    // 移動ベクトル
	float radius_;    // 当たり判定用の半径
	int damage_;     // 与えるダメージ量
	int knockback_; // ノックバック力
	int ownerId_;   // 攻撃オブジェクトの所有者ID
	bool isActive_; // 有効状態

};

