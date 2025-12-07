#pragma once
#include "../base/Math/MyMath.h"
#include <d3d12.h>
#include <wrl/client.h>

namespace TDEngine {

	// 定数バッファ用データ構造
	struct ConstBufferDataWorldTransform {
		MyMath::Matrix4x4 matWorld; // ワールド行列
		MyMath::Matrix4x4 WVP;      // ViewProjection込み行列 (シェーダーで必要)
	};

	/// <summary>
	/// ワールド変換データ
	/// </summary>
	class WorldTransform {
		MyMath::Vector3 scale_ = { 1.0f, 1.0f, 1.0f };
		MyMath::Vector3 rotation_ = { 0.0f, 0.0f, 0.0f };
		MyMath::Vector3 translation_ = { 0.0f, 0.0f, 0.0f };

		MyMath::Matrix4x4 matWorld_ = MyMath::Identity4x4();

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

		// 互換性用: UpdateWorldMatrixという名前で呼ばれてもUpdateMatrixに流す
		void UpdateWorldMatrix() { UpdateMatrix(); }
		// 引数ありで呼ばれる場合の互換性 (worldTransform_.UpdateWorldMatrix(worldTransform_) の対策)
		void UpdateWorldMatrix(const WorldTransform& /*dummy*/) { UpdateMatrix(); }

		/// <summary>
		/// 定数バッファにデータを転送する
		/// </summary>
		void TransferMatrix();
	};

}