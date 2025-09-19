#pragma once
#include <string>

// プレイヤークラス(基底クラス)
class Player
{
protected:

	// 操作するコントローラID
	int ctrlId_;

public:

	// コンストラクタ
	Player(int id) : ctrlId_(id) {}		// コントローラIDをセット

	// デストラクタ
	virtual ~Player() = default;

	// 純粋仮想関数でクラス設計実施

	// 初期化処理
	virtual void Init(void) = 0;

	// 更新処理
	virtual void Update(void) = 0;

	virtual void Draw(void) = 0;

	// 攻撃処理
	virtual void Attack(void) = 0;

	// ステートメントは後ほど実装

	// character名を取得
	virtual std::string GetCharacterName(void) const = 0;
};