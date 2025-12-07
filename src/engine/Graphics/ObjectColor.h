#pragma once
#include "../base/Math/MyMath.h"
#include <d3d12.h>
#include <wrl/client.h>

namespace TDEngine {

	// 定数バッファ用データ構造 (Model::Materialと合わせる)
	struct ConstBufferDataColor {
		MyMath::Vector4 color;
		int32_t enableLighting;
		float shininess;
		float padding[2];
		MyMath::Matrix4x4 uvTransform;
	};

	class ObjectColor {
	public:
		/// <summary>
		/// 初期化
		/// </summary>
		void Initialize();

		/// <summary>
		/// 色を設定
		/// </summary>
		void SetColor(const MyMath::Vector4& color);

		/// <summary>
		/// 色を取得
		/// </summary>
		const MyMath::Vector4& GetColor() const { return color_; }

		// 描画時にGPUアドレスを取得するためのゲッター
		D3D12_GPU_VIRTUAL_ADDRESS GetGPUVirtualAddress() const;

	private:
		MyMath::Vector4 color_ = { 1.0f, 1.0f, 1.0f, 1.0f };

		// 定数バッファ
		Microsoft::WRL::ComPtr<ID3D12Resource> constBuff_;
		// マッピング済みアドレス
		ConstBufferDataColor* constMap_ = nullptr;
	};

}