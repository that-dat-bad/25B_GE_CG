#pragma once
#include <d3d12.h>
#include <wrl/client.h>
#include <cstdint>
#include <array>
#include "BlendMode.h"
#include "../../engine/base/Math/MyMath.h"
#include <memory> 

class DirectXCommon;
class Camera;
using namespace MyMath;

struct DirectionalLight {
	Vector4 color;
	Vector3 direction;
	float intensity;
};

struct PointLight {
	Vector4 color;
	Vector3 position;
	float intensity;
	float radius;
	float decay;
	float padding[2];
};

struct SpotLight {
	Vector4 color;
	Vector3 position;
	float intensity;
	Vector3 direction;
	float distance;
	float decay;
	float cosAngle;
	float cosFalloffStart;
	float padding;
};

struct LightingSettings {
	int32_t shadingModel; // 0: Lambert, 1: Half-Lambert
	int32_t specularModel; // 0: None, 1: Phong, 2: Blinn-Phong
	int32_t lightType; // 0: Directional, 1: Point, 2: Both
	float padding;
	Vector3 cameraPosition;
	float padding2;
};

class Object3dCommon
{
public:
	static Object3dCommon* GetInstance();

	void Initialize(DirectXCommon* dxCommon);

	void SetupCommonState();

	void SetBlendMode(BlendMode mode);

	void SetShadingModel(int32_t model) { lightingSettingsData->shadingModel = model; }
	void SetSpecularModel(int32_t model) { lightingSettingsData->specularModel = model; }
	void SetLightType(int32_t type) { lightingSettingsData->lightType = type; }
	void SetCameraPosition(const Vector3& position) { lightingSettingsData->cameraPosition = position; }

	DirectXCommon* GetDirectXCommon() { return dxCommon_; }

	void SetDefaultCamera(Camera* camera) { defaultCamera_ = camera; }

	Camera* GetDefaultCamera() const { return defaultCamera_; }
	ID3D12Resource* GetDirectionalLightResource() { return directionalLightResource_.Get(); }
	ID3D12Resource* GetPointLightResource() { return pointLightResource_.Get(); }
	ID3D12Resource* GetSpotLightResource() { return spotLightResource_.Get(); }
	ID3D12Resource* GetLightingSettingsResource() { return lightingSettingsResource_.Get(); }
	DirectionalLight* GetDirectionalLightData() { return directionalLightData; }
	PointLight* GetPointLightData() { return pointLightData; }
	SpotLight* GetSpotLightData() { return spotLightData; }
	~Object3dCommon() = default;
private:
	Object3dCommon() = default;
	Object3dCommon(const Object3dCommon&) = delete;
	Object3dCommon& operator=(const Object3dCommon&) = delete;

	static std::unique_ptr<Object3dCommon> instance;

	DirectXCommon* dxCommon_ = nullptr;
	Camera* defaultCamera_ = nullptr;
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;
	std::array<Microsoft::WRL::ComPtr<ID3D12PipelineState>, static_cast<size_t>(BlendMode::kCountOf)> graphicsPipelineStates_;
	Microsoft::WRL::ComPtr<ID3D12Resource> directionalLightResource_;
	Microsoft::WRL::ComPtr<ID3D12Resource> pointLightResource_;
	Microsoft::WRL::ComPtr<ID3D12Resource> spotLightResource_;

	Microsoft::WRL::ComPtr<ID3D12Resource> lightingSettingsResource_;

	DirectionalLight* directionalLightData = nullptr;
	PointLight* pointLightData = nullptr;
	SpotLight* spotLightData = nullptr;
	LightingSettings* lightingSettingsData = nullptr;
	void CreateRootSignature(DirectXCommon* dxCommon);

	void CreateGraphicsPipeline(DirectXCommon* dxCommon);
};

