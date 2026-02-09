#pragma once
#include <d3d12.h>
#include <wrl/client.h>
#include <string>
#include <vector>
#include "../base/Math/MyMath.h"
#include "DirectXCommon.h"
#include "BlendMode.h"

using namespace MyMath;

class Object3dCommon;
class Model;
class Camera;

#ifdef Object3d
#undef Object3d
#endif

class Object3d
{
public:
	
	struct TransformationMatrix {
		Matrix4x4 WVP;
		Matrix4x4 World;
		Matrix4x4 WorldInverseTranspose;
	};
	struct DirectionalLight {
		Vector4 color;
		Vector3 direction;
		float intensity;
	};

public: 
	void Initialize(Object3dCommon* object3dCommon);
	void Update();
	void Draw();

	void SetScale(const Vector3& scale) { transform_.scale = scale; }
	void SetRotate(const Vector3& rotate) { transform_.rotate = rotate; }
	void SetTranslate(const Vector3& translate) { transform_.translate = translate; }
	void SetModel(Model* model) { model_ = model; }
	void SetModel(const std::string& filePath);
	void SetCamera(Camera* camera) { camera_ = camera; }
	void SetBlendMode(BlendMode mode) { blendMode_ = mode; }

	Vector3 GetScale() const { return transform_.scale; }
	Vector3 GetRotate() const { return transform_.rotate; }
	Vector3 GetTranslate() const { return transform_.translate; }
	Model* GetModel() const { return model_; }
	
	// Add this to allow getting world matrix
	Matrix4x4 GetWorldMatrix() const;



private: 
	Object3dCommon* object3dCommon_ = nullptr;
	Model* model_ = nullptr;
	Camera* camera_ = nullptr;

	Transform transform_;

	Microsoft::WRL::ComPtr<ID3D12Resource> transformationMatrixResource_ = nullptr;
	TransformationMatrix* transformationMatrixData_ = nullptr;

	BlendMode blendMode_ = BlendMode::kNormal;
};
