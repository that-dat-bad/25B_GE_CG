#pragma once
class DirectXCommon;
/// <summary>
/// 3Dモデルの共通設定やリソースを管理するクラス
/// </summary>
class ModelCommon
{
public:
	/// <summary>
	/// 初期化処理
	/// </summary>
	/// <param name="dxCommon">DirectX共通クラスのポインタ</param>
	void Initialize(DirectXCommon* dxCommon);

	/// <summary>
	/// DirectX共通クラスのポインタを取得
	/// </summary>
	/// <returns>DirectX共通クラスポインタ</returns>
	DirectXCommon* GetDirectXCommon() { return dxCommon_; }
private:
	DirectXCommon* dxCommon_;
};

