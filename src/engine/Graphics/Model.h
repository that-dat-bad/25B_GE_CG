#pragma once
#include <d3d12.h>
#include <wrl/client.h>
#include <string>
#include "../base/Math/MyMath.h"
#include<vector>
#include "WorldTransform.h"
#include "Camera.h"
using namespace MyMath;


namespace TDEngine {
	class ModelCommon;
	class Model
	{
	public:
		static void LoadFromOBJ(const std::string& modelName);
		static Model* CreateFromOBJ(const std::string& modelName, bool smoothing = false);
		void Initialize(ModelCommon* modelCommon, const std::string& directorypath, const std::string& filename);
		void Draw();
		void Draw(const WorldTransform& worldTransform, const Camera& camera);
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
		void SetAlpha(float alpha);

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

}