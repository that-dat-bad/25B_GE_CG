#pragma once
#include <d3d12.h>
#include <wrl/client.h>
#include <string>
#include <vector>
#include "../base/Math/MyMath.h"
#include "DirectXCommon.h"

// 名前空間の利用
using namespace MyMath;

// 前方宣言
class Object3dCommon;

class Object3d
{
public:
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
		float shininess;
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

public: // メンバ関数
	// 初期化
	void Initialize(Object3dCommon* object3dCommon);
	// 更新
	void Update();
	// 描画
	void Draw();

	// セッター (外部から座標などを変更するため)
	void SetScale(const Vector3& scale) { transform_.scale = scale; }
	void SetRotate(const Vector3& rotate) { transform_.rotate = rotate; }
	void SetTranslate(const Vector3& translate) { transform_.translate = translate; }

	// 静的関数 (.mtl / .obj 読み込み)
	static MaterialData LoadMaterialTemplate(const std::string& directoryPath, const std::string& filename);
	static ModelData LoadObjFile(const std::string& directoryPath, const std::string& filename);

private: // メンバ変数
	Object3dCommon* object3dCommon_ = nullptr;

	// モデルデータ (CPU側)
	ModelData modelData_;

	// トランスフォーム
	Transform transform_;
	Transform cameraTransform_;

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

	//--座標変換行列--//
	Microsoft::WRL::ComPtr<ID3D12Resource> transformationMatrixResource_ = nullptr;
	TransformationMatrix* transformationMatrixData_ = nullptr;

	//--平行光源--//
	Microsoft::WRL::ComPtr<ID3D12Resource> directionalLightResource_ = nullptr;
	DirectionalLight* directionalLightData_ = nullptr;
};