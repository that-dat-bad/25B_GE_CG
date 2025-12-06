#pragma once
#include <TDEngine.h>

class BackGround {
public:
	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="model">モデル</param>
	/// <param name="position">初期座標</param>

	void Initialize(TDEngine::Model* model, TDEngine::Camera* camera, const TDEngine::Vector3& position);

	/// <summary>
	/// 更新
	/// </summary>

	void Update();

	/// <summary>
	/// 描画
	/// </summary>

	void Draw();

private:
	// ワールド変換データ
	TDEngine::WorldTransform worldTransform_;
	// モデル
	TDEngine::Model* model_ = nullptr;

	// カメラ
	TDEngine::Camera* camera_ = nullptr;
};
