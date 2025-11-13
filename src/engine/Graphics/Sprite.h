#pragma once
#include"../base/Math/MyMath.h"
using namespace MyMath;
#include <d3d12.h>
#include <cstdint>
#include <wrl/client.h>

class SpriteCommon;
class DirectXCommon;

class Sprite
{
public:
	void Initialize(SpriteCommon* spriteCommon, DirectXCommon* dxCommon);

	void Update();

private:

	SpriteCommon* spriteCommon_ = nullptr;


	//---頂点データ---//
	struct VertexData {
		Vector4 position;
		Vector2 texcoord;
	};

	//バッファリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexBuffer_ = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> indexBuffer_ = nullptr;

	//バッファリソース内のデータを指すポインタ
	VertexData* vertexData_ = nullptr;
	uint32_t* indexData_ = nullptr;

	D3D12_VERTEX_BUFFER_VIEW vertexBufferView{};
	D3D12_INDEX_BUFFER_VIEW indexBufferView{};


	//---マテリアルデータ---//
	struct Material {
		Vector4 color;
		int32_t enableLighting;
		float shininess;
		float padding[3];
		Matrix4x4 uvTransform; // UV変換行列
	};

	//バッファリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource_ = nullptr;
	Material* materialData_ = nullptr;


	//---座標変換行列---//
	struct TransformationMatrix {
		Matrix4x4 WVP;
		Matrix4x4 World;
	};

	//バッファリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> wvpResource_ = nullptr;
	//バッファリソース内のデータを指すポインタ
	TransformationMatrix* transformationMatrixData_ = nullptr;

	Transform transform_ = { {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f} };
};

