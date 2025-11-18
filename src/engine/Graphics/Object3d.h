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

	void Initialize(Object3dCommon* object3dCommon, DirectXCommon* dxCommon);
	void Update(const Transform& cameraTransform);
	std::map<std::string, MaterialData> LoadMaterialTemplates(const std::string& directoryPath, const std::string& filename);

	void Draw(DirectXCommon* dxCommon);
private:

	Object3dCommon* object3dCommon_ = nullptr;
	DirectXCommon* dxCommon_ = nullptr;




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

