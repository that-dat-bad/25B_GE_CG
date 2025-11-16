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

	void Draw(DirectXCommon* dxCommon, D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandle);
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
		Matrix4x4 WVP;
		Vector4 color;
	};

	//バッファリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource_ = nullptr;
	Material* materialData_ = nullptr;


	Transform transform_ = { {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f} };
};

