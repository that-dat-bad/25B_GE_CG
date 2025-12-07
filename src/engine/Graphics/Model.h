#pragma once
#include <d3d12.h>
#include <wrl/client.h>
#include <string>
#include "../base/Math/MyMath.h"
#include <vector>

using namespace MyMath;

namespace TDEngine {

	class ModelCommon;
	class WorldTransform;
	class Camera;
	class ObjectColor;

	class Model
	{
	public:
		static Model* CreateFromOBJ(const std::string& modelName, bool smoothing = false);
		static void LoadFromOBJ(const std::string& modelName);

		void Initialize(ModelCommon* modelCommon, const std::string& directorypath, const std::string& filename);

		// 既存のDraw (引数なし版)
		void Draw();

		// これにより model->Draw(wt, cam, &col) の呼び出しが可能になります
		void Draw(const WorldTransform& worldTransform, const Camera& camera, const ObjectColor* objectColor = nullptr);

		void SetAlpha(float alpha);

		struct VertexData {
			Vector4 position;
			Vector2 texcoord;
			Vector3 normal;
		};
		// 定数バッファの構造体 (ObjectColorと構造を合わせる必要があります)
		struct Material {
			Vector4 color;
			int32_t enableLighting;
			float shininess;
			float padding[2];
			Matrix4x4 uvTransform;
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
		ModelData modelData_;
		Microsoft::WRL::ComPtr<ID3D12Resource> vertexBuffer_ = nullptr;
		VertexData* vertexData_ = nullptr;
		D3D12_VERTEX_BUFFER_VIEW vertexBufferView_{};
		Microsoft::WRL::ComPtr<ID3D12Resource> materialResource_ = nullptr;
		Material* materialData_ = nullptr;
	};

}