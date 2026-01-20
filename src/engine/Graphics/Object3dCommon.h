#pragma once
#include <d3d12.h>
#include <wrl/client.h>
#include <cstdint>
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

struct LightingSettings {
	int32_t lightingModel; // 0: Lambert, 1: Half-Lambert
	float padding[3];
};

class Object3dCommon
{
public:
	static Object3dCommon* GetInstance();

	void Initialize(DirectXCommon* dxCommon);

	//共通描画設定
	void SetupCommonState();

	DirectXCommon* GetDirectXCommon() { return dxCommon_; }

	//アクセッサ
	//セッター
	void SetDefaultCamera(Camera* camera) { defaultCamera_ = camera; }

	//ゲッター
	Camera* GetDefaultCamera() const { return defaultCamera_; }
	ID3D12Resource* GetDirectionalLightResource() { return directionalLightResource_.Get(); }
	ID3D12Resource* GetLightingSettingsResource() { return lightingSettingsResource_.Get(); }
	DirectionalLight* GetDirectionalLightData() { return directionalLightData; }
	~Object3dCommon() = default;
private:
	Object3dCommon() = default;
	Object3dCommon(const Object3dCommon&) = delete;
	Object3dCommon& operator=(const Object3dCommon&) = delete;

	static std::unique_ptr<Object3dCommon> instance;

	DirectXCommon* dxCommon_ = nullptr;
	Camera* defaultCamera_ = nullptr;
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineState_;
	Microsoft::WRL::ComPtr<ID3D12Resource> directionalLightResource_;
	Microsoft::WRL::ComPtr<ID3D12Resource> lightingSettingsResource_;

	DirectionalLight* directionalLightData = nullptr;
	LightingSettings* lightingSettingsData = nullptr;
	//ルートシグネチャの作成
	void CreateRootSignature(DirectXCommon* dxCommon);

	//グラフィックパイプラインの生成
	void CreateGraphicsPipeline(DirectXCommon* dxCommon);
};
