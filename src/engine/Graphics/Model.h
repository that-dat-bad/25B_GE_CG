#pragma once
#include <d3d12.h>
#include <wrl/client.h>
#include <string>
#include "../base/Math/MyMath.h"
#include<vector>
using namespace MyMath;
class ModelCommon;
class Model
{
public:
	void Initialize(ModelCommon* modelCommon,const std::string& directorypath,const std::string& filename);
	void Draw();

	void SetShininess(float shininess) { materialData_->shininess = shininess; }
	float GetShininess() const { return materialData_->shininess; }

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


	static MaterialData LoadMaterialTemplate(const std::string& directoryPath, const std::string& filename);
	static ModelData LoadObjFile(const std::string& directoryPath, const std::string& filename);
private:

	ModelCommon* modelCommon_ = nullptr;

	// モデルデータ
	ModelData modelData_;

	//--頂点データ--//
	// バッファリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexBuffer_ = nullptr;
	// バッファリソース内のデータを指すポインタ
	VertexData* vertexData_ = nullptr;
	// バッファビュー
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_{};

	//--マテリアル--//
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource_ = nullptr;
	Material* materialData_ = nullptr;
};

