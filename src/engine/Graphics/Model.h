#pragma once
#include <d3d12.h>
#include <wrl/client.h>
#include "../base/Math/MyMath.h"
#include <string>
#include <vector>
using namespace MyMath;
class ModelCommon;

class Model
{

private:

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

	struct MaterialData {
		std::string textureFilePath;
		std::string name;
		uint32_t textureIndex = 0;
	};

	struct ModelData {
		std::string name;
		MaterialData material;
	};
public:
	void Initialize(ModelCommon* modelCommon);

	ModelData LoadObjFile(const std::string& directoryPath, const std::string& filename);

private:
	//modelCommonのポインタ
	ModelCommon* modelCommon_ = nullptr;
	//objファイルのデータ
	ModelData modelData_;
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
};

