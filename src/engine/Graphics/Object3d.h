#pragma once
#include <string>
#include <vector>
#include <map>
#include "../base/Math/MyMath.h"
#include "DirectXCommon.h"
using namespace MyMath;
#include <d3d12.h>

class Object3dCommon;

class Object3d
{
public:

	struct MaterialData {
		std::string textureFilePath;
		std::string name;
		uint32_t textureIndex = 0;
	};
	struct VertexData {
		Vector4 position;
		Vector2 texcoord;
		Vector3 normal;
	};
	struct Material {
		Vector4 color;
		int32_t enableLighting;
		float shininess;
		float padding[2];
		Matrix4x4 uvTransform; // UV変換行列
	};
	struct TransformationMatrix {
		Matrix4x4 WVP;
		Matrix4x4 World;
	};

	struct MeshObject {
		std::string name;
		std::vector<VertexData> vertices;
		MaterialData material; // このメッシュに適用されるマテリアル
		Transform transform; // このメッシュ固有のSRT
		Transform uvTransform; // メッシュ固有のUV変換
		Microsoft::WRL::ComPtr<ID3D12Resource> vertexBuffer;
		D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
		Microsoft::WRL::ComPtr<ID3D12Resource> materialResource;
		Material* materialData = nullptr;
		Microsoft::WRL::ComPtr<ID3D12Resource> wvpResource;
		TransformationMatrix* wvpData = nullptr;
		int textureAssetIndex = 0; // このメッシュが使用するテクスチャのインデックス
		bool hasUV = false; // このメッシュがUVを持つか
	};
	struct ModelData {
		std::string name;
		std::vector<MeshObject> meshes;
		MaterialData material;
	};

	struct DirectionalLight {
		Vector4 color;
		Vector3 direction;
		float intensity;
	};

	void Initialize(Object3dCommon* object3dCommon, DirectXCommon* dxCommon);
	void Update(const Transform& cameraTransform);
	std::map<std::string, MaterialData> LoadMaterialTemplates(const std::string& directoryPath, const std::string& filename);
	ModelData LoadObjFile(const std::string& directoryPath, const std::string& filename);
private:

	Object3dCommon* object3dCommon_ = nullptr;
	DirectXCommon* dxCommon_ = nullptr;

	//objファイルのデータ
	ModelData modelData_;

	////--頂点データ--//
	////バッファリソース
	//Microsoft::WRL::ComPtr<ID3D12Resource> vertexBuffer_ = nullptr;
	////バッファリソース内のデータを指すポインタ
	//VertexData* vertexData_ = nullptr;
	////バッファリソースの使い道を補足するバッファビュー
	//D3D12_VERTEX_BUFFER_VIEW vertexBufferView_;

	////--マテリアル--//
	////バッファリソース
	//Microsoft::WRL::ComPtr<ID3D12Resource> materialResource_ = nullptr;
	////バッファリソース内のデータを指すポインタ
	//Material* materialData_ = nullptr;

	////--座標変換行列--//
	////バッファリソース
	//Microsoft::WRL::ComPtr<ID3D12Resource> transformationMatrixResource_ = nullptr;
	////バッファリソース内のデータを指すポインタ
	//TransformationMatrix* transformationMatrixData_ = nullptr;

	//--平行光源--//
	//バッファリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> directionalLightResource_ = nullptr;
	//バッファリソース内のデータを指すポインタ
	DirectionalLight* directionalLightData_ = nullptr;

	Transform transform_ = {};
	Transform cameraTransform__{};
};

