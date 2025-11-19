#pragma once
#include <d3d12.h>
#include <wrl/client.h>
#include <string>
#include <vector>
#include "../base/Math/MyMath.h"
#include "DirectXCommon.h"

using namespace MyMath;

class Object3dCommon;
class Model;

class Object3d
{
public:
	
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

	// セッター
	void SetScale(const Vector3& scale) { transform_.scale = scale; }
	void SetRotate(const Vector3& rotate) { transform_.rotate = rotate; }
	void SetTranslate(const Vector3& translate) { transform_.translate = translate; }
	void SetModel(Model* model) { model_ = model; }
	void SetModel(const std::string& filePath);

	// ゲッター
	Vector3 GetScale() const { return transform_.scale; }
	Vector3 GetRotate() const { return transform_.rotate; }
	Vector3 GetTranslate() const { return transform_.translate; }

	// 静的関数 (.mtl / .obj 読み込み)
	//static MaterialData LoadMaterialTemplate(const std::string& directoryPath, const std::string& filename);
	//static ModelData LoadObjFile(const std::string& directoryPath, const std::string& filename);

private: 
	Object3dCommon* object3dCommon_ = nullptr;
	Model* model_ = nullptr;

	//// モデルデータ
	//ModelData modelData_;

	// トランスフォーム
	Transform transform_;
	Transform cameraTransform_;

	////--頂点データ--//
	//// バッファリソース
	//Microsoft::WRL::ComPtr<ID3D12Resource> vertexBuffer_ = nullptr;
	//// バッファリソース内のデータを指すポインタ
	//VertexData* vertexData_ = nullptr;
	//// バッファビュー
	//D3D12_VERTEX_BUFFER_VIEW vertexBufferView_{};

	////--マテリアル--//
	//Microsoft::WRL::ComPtr<ID3D12Resource> materialResource_ = nullptr;
	//Material* materialData_ = nullptr;

	//--座標変換行列--//
	Microsoft::WRL::ComPtr<ID3D12Resource> transformationMatrixResource_ = nullptr;
	TransformationMatrix* transformationMatrixData_ = nullptr;

	//--平行光源--//
	Microsoft::WRL::ComPtr<ID3D12Resource> directionalLightResource_ = nullptr;
	DirectionalLight* directionalLightData_ = nullptr;
};