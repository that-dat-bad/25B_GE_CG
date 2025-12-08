#pragma once
#include "../base/Math/MyMath.h"
#include <d3d12.h>
#include <wrl/client.h>

// 定数バッファ用データ構造（アライメント調整済み）
struct ConstBufferDataWorldTransform {
	MyMath::Matrix4x4 matWorld; // ローカル -> ワールド変換行列
};

/// <summary>
/// ワールド変換データ
/// </summary>
struct WorldTransform {
	// スケール
	MyMath::Vector3 scale = { 1.0f, 1.0f, 1.0f };
	// 回転
	MyMath::Vector3 rotation = { 0.0f, 0.0f, 0.0f };
	// 平行移動
	MyMath::Vector3 translation = { 0.0f, 0.0f, 0.0f };

	// ワールド変換行列
	MyMath::Matrix4x4 matWorld = MyMath::Identity4x4();

	// 親となるワールド変換へのポインタ
	const WorldTransform* parent_ = nullptr;

	// 定数バッファ
	Microsoft::WRL::ComPtr<ID3D12Resource> constBuff_;
	// マッピング済みアドレス
	ConstBufferDataWorldTransform* constMap = nullptr;

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize();

	/// <summary>
	/// 行列を更新する
	/// </summary>
	void UpdateMatrix();

	/// <summary>
	/// 定数バッファにデータを転送する
	/// </summary>
	void TransferMatrix();
};