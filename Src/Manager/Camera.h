#pragma once
#include <DxLib.h>

class Camera
{

public:

	// カメラの初期位置
	static constexpr VECTOR DEFAULT_POS = { 0.f,745.f,-1570.f };

	// カメラの初期角度（ラジアン）
	static constexpr VECTOR DEFAULT_ANGLES = {
		30.f * DX_PI_F / 180.f, 0.f, 0.f
	};

	// カメラのクリップ範囲
	static constexpr float VIEW_NEAR = 1.f;		// ニアクリップ
	static constexpr float VIEW_FAR = 30000.f;	// ファークリップ

	// カメラモード
	enum class MODE
	{
		NONE,
		FIXED_POINT,	// 固定カメラ（本作では「平均注視・X軸回転」の追従に利用）
		FREE,			// 自由カメラ（デバッグ）
	};

	// コンストラクタ
	Camera(void);

	// デストラクタ
	~Camera(void);

	// 初期化
	void Init(void);

	// 更新（FIXED_POINT時のみ、プレイヤー平均注視＋X軸回転で追従）
	void Update(void);

	// 描画前のカメラ設定
	void SetBeforeDraw(void);
	void SetBeforeDrawFixedPoint(void);
	void SetBeforeDrawFree(void);

	// デバッグ描画
	void DrawDebug(void);

	// 破棄
	void Release(void);

	// 位置の取得
	const VECTOR& GetPos(void) const;

	// 角度の取得
	const VECTOR& GetAngles(void) const;

	// カメラモードの変更
	void ChangeMode(MODE mode);

	// 自由カメラ移動（デバッグ）
	void MoveXYZDirection(void);

	// 追加: ゲーム開始用の安定リセット
	// - pitchDeg: X軸回転（度数法）
	// - baseDistance: 初期距離（オートズームは数フレーム後から有効化）
	// - enableAutoZoom: オートズームを使うか
	// - zoomSpreadScale: 散開→距離の寄与係数
	void ResetForGame(float pitchDeg = 30.0f, float baseDistance = 1400.0f,
		bool enableAutoZoom = true, float zoomSpreadScale = 1.5f);

private:

	// 内部ヘルパ
	static inline VECTOR LerpV(const VECTOR& a, const VECTOR& b, float t)
	{
		return { a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t, a.z + (b.z - a.z) * t };
	}

	// カメラモード
	MODE mode_;

	// カメラの位置
	VECTOR pos_;

	// カメラの角度（x=ピッチのみ使用）
	VECTOR angles_;

	// 追従設定
	bool   followPlayers_ = false;
	bool   autoZoom_ = true;
	float  baseDistance_ = 1400.0f;
	float  zoomSpreadScale_ = 1.5f;
	float  minDistance_ = 800.0f;
	float  maxDistance_ = 2600.0f;
	float  lerpFactor_ = 0.12f;

	// オートズームの滑らかな立ち上げ
	int    zoomRampFrames_ = 30; // 最初のNフレームは固定距離
	int    zoomRampCounter_ = 0;

	// 直近ターゲット（スムージング）
	VECTOR lastTarget_ = { 0.0f, 0.0f, 0.0f };
	bool   hasLastTarget_ = false;
};