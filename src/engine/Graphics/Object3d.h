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
class Camera;

class Object3d
{
public:
	
	static Object3d* Create();

	struct TransformationMatrix {
		Matrix4x4 WVP;
		Matrix4x4 World;
	};
	struct DirectionalLight {
		Vector4 color;
		Vector3 direction;
		float intensity;
	};

	struct Material {
		Vector4 color;
		int32_t enableLighting;
		float padding[3];
		Matrix4x4 uvTransform;
	};

public:

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
	void SetCamera(Camera* camera) { camera_ = camera; }
	void SetColor(const Vector4& color) {
		if (materialData_) {
			materialData_->color = color;
		}
	}
	// ゲッター
	Vector3 GetScale() const { return transform_.scale; }
	Vector3 GetRotate() const { return transform_.rotate; }
	Vector3 GetTranslate() const { return transform_.translate; }



private: 
	Object3dCommon* object3dCommon_ = nullptr;
	Model* model_ = nullptr;
	Camera* camera_ = nullptr;

	// トランスフォーム
	Transform transform_;

	//--座標変換行列--//
	Microsoft::WRL::ComPtr<ID3D12Resource> transformationMatrixResource_ = nullptr;
	TransformationMatrix* transformationMatrixData_ = nullptr;

	//--平行光源--//
	Microsoft::WRL::ComPtr<ID3D12Resource> directionalLightResource_ = nullptr;
	DirectionalLight* directionalLightData_ = nullptr;

	//マテリアル(色)用リソース
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource_ = nullptr;
	Material* materialData_ = nullptr;
};