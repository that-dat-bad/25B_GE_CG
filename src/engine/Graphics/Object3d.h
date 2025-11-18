#pragma once
#include <d3d12.h>
#include <wrl/client.h>
#include<string>
#include<vector>
#include "../base/Math/MyMath.h"
#include"DirectXCommon.h"
using namespace MyMath;
class Object3dCommon;
class Object3d
{
private:
	struct VertexData {
		Vector4 position;
		Vector2 texcoord;
		Vector3 normal;
	};
	struct MaterialData
	{
		std::string textureFilePath;
		uint32_t textureIndex;
	};
	struct ModelData
	{
		std::vector<VertexData> vertices;
		MaterialData material;
	};
	struct Material {
		Vector4 color;
		int32_t enableLighting;
		float padding[2];
		Matrix4x4 uvTransform; // UV変換行列
	};
	struct TransformationMatrix {
		Matrix4x4 WVP;
		Matrix4x4 World;
	};
	struct DirectionalLight {
		Vector4 color;
		Vector3 direction;
		float intensity;
	};
public:
	void Initialize(Object3dCommon* object3dCommon);
	//.mtlファイルの読み取り
	static MaterialData LoadMaterialTemplate(const std::string& directoryPath, const std::string& filename);
	//.objファイルの読み取り
	static ModelData LoadObjFile(const std::string& directoryPath, const std::string& filename);
private:
	Object3dCommon* object3dCommon_ = nullptr;
	//objファイルのデータ
	ModelData modelData_;
	
	Transform transform_;
	Transform cameraTransform_;

	//--頂点データ--//
	//バッファリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexBuffer_ = nullptr;
	//バッファリソース内のデータを指すポインタ
	VertexData* vertexData_ = nullptr;
	//バッファリソースの使い道を補足するバッファビュー
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_;
	//--マテリアル--//
	//バッファリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource_ = nullptr;
	//バッファリソース内のデータを指すポインタ
	Material* materialData_ = nullptr;
	//--マテリアル--//
	//バッファリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource_ = nullptr;
	//バッファリソース内のデータを指すポインタ
	Material* materialData_ = nullptr;
	//--座標変換行列--//
	//バッファリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> transformationMatrixResource_ = nullptr;
	//バッファリソース内のデータを指すポインタ
	TransformationMatrix* transformationMatrixData_ = nullptr;
	//--平行光源--//
	//バッファリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> directionalLightResource_ = nullptr;
	//バッファリソース内のデータを指すポインタ
	DirectionalLight* directionalLightData_ = nullptr;
};

