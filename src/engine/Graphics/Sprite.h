#pragma once
#include"../base/Math/MyMath.h"
using namespace MyMath;
#include <d3d12.h>
#include <cstdint>
#include <wrl/client.h>
#include<string>

class SpriteCommon;
class DirectXCommon;

class Sprite
{
public:
	void Initialize(SpriteCommon* spriteCommon, DirectXCommon* dxCommon,std::string textureFilePath);

	void Update();

	void Draw(DirectXCommon* dxCommon, D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandle);

	void ChangeTexture(std::string textureFilePath);

	//--アクセッサ--//
	//---セッター---//
	void SetPosition(const Vector2& pos) { transform_.translate = { pos.x, pos.y, 0.0f }; }
	void SetRotation(float rot) { transform_.rotate.z = rot; }
	void SetSize(const Vector2& size) { transform_.scale = { size.x, size.y, 1.0f }; }
	void SetColor(const Vector4& color) { materialData_->color = color; }

	//---ゲッター---//
	Vector2 GetPosition() const { return { transform_.translate.x, transform_.translate.y }; }
	float GetRotation() const { return transform_.rotate.z; }
	Vector2 GetSize() const { return { transform_.scale.x, transform_.scale.y }; }
	Vector4 GetColor() const { return materialData_->color; }

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

	uint32_t textureIndex_ = 0;
};

