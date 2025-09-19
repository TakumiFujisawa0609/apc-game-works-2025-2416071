#pragma once

class StageManager
{
public:
	// コンストラクタ
	StageManager(void);

	// デストラクタ
	virtual ~StageManager(void);

	// 初期化処理
	virtual void Init(void);

	// 更新ステップ
	virtual void Update(void) ;

	// 描画処理
	virtual void Draw(void) ;

	// 解放処理
	virtual void Release(void);

	// プレイ人数を取得
	int GetPlayerNum(void) const { return playerNum_; }

private:

	// ここにメンバ変数を追加していく

protected:

	// 選択された人数
	int playerNum_;

	// ステージのオブジェクト
	int modelHandle_;
};

