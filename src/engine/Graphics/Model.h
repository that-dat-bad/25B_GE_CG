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
	struct Node {
		Matrix4x4 localMatrix;
		std::string name;
		std::vector<Node> children;
	};
	struct ModelData
	{
		std::vector<VertexData> vertices;
		MaterialData material;
		Node rootNode;
	};


	static ModelData LoadModelFile(const std::string& directoryPath, const std::string& filename);
	ModelData GetModelData() const { return modelData_; }
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

