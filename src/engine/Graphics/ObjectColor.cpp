#include "ObjectColor.h"
#include "DirectXCommon.h"
#include <cassert>

namespace TDEngine {

	void ObjectColor::Initialize() {
		// 定数バッファの生成
		constBuff_ = DirectXCommon::GetInstance()->CreateBufferResource(sizeof(ConstBufferDataColor));

		// マッピング
		HRESULT hr = constBuff_->Map(0, nullptr, reinterpret_cast<void**>(&constMap_));
		assert(SUCCEEDED(hr));

		// 初期値設定
		color_ = { 1.0f, 1.0f, 1.0f, 1.0f };
		if (constMap_) {
			constMap_->color = color_;
			constMap_->enableLighting = 1; // デフォルトでライティング有効
			constMap_->shininess = 1.0f;
			constMap_->uvTransform = MyMath::Identity4x4();
		}
	}

	void ObjectColor::SetColor(const MyMath::Vector4& color) {
		color_ = color;
		if (constMap_) {
			constMap_->color = color_;
		}
	}

	D3D12_GPU_VIRTUAL_ADDRESS ObjectColor::GetGPUVirtualAddress() const {
		return constBuff_->GetGPUVirtualAddress();
	}

}