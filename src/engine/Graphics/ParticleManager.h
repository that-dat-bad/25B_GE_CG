#pragma once
#include <d3d12.h>
#include <wrl/client.h>
#include <string>
#include <vector>
#include <list>
#include <unordered_map>
#include <random>

#include "../base/Math/MyMath.h"
#include "DirectXCommon.h"
#include "SrvManager.h"
#include "BlendMode.h"
#include <array>

using namespace MyMath;

struct Particle {
	Transform transform;
	Vector3 velocity;
	Vector3 acceleration; 
	Vector4 color;
	float lifeTime;
	float currentTime;
};

struct ParticleParameters {
	Vector3 minVelocity = { -1.0f, -1.0f, -1.0f };
	Vector3 maxVelocity = { 1.0f, 1.0f, 1.0f };
	Vector4 minColor = { 1.0f, 1.0f, 1.0f, 1.0f };
	Vector4 maxColor = { 1.0f, 1.0f, 1.0f, 1.0f };
	float minLifeTime = 1.0f;
	float maxLifeTime = 3.0f;
	Vector3 acceleration = { 0.0f, 0.0f, 0.0f }; 
};

struct ParticleInstancingData {
	Matrix4x4 WVP;
	Vector4 color;
};

// AABB (Axis Aligned Bounding Box)
struct AABB {
	Vector3 min; 
	Vector3 max; 
};

struct AccelerationField {
	Vector3 acceleration; 
	AABB area;            
};

struct ParticleGroup {
	std::string textureFilePath;
	uint32_t textureSrvIndex;

	std::list<Particle> particles;

	uint32_t instancingSrvIndex;
	Microsoft::WRL::ComPtr<ID3D12Resource> instancingResource;
	UINT instanceCount; 
	ParticleInstancingData* instancingDataPtr = nullptr; 
	BlendMode blendMode = BlendMode::kNormal;
};

#include <memory> 

class ParticleManager
{
public: 
	static ParticleManager* GetInstance();

	void Initialize(DirectXCommon* dxCommon, SrvManager* srvManager);
	void Update();
	void Draw();
	void Finalize();

	void CreateParticleGroup(const std::string& name, const std::string& textureFilePath);

	void Emit(const std::string& name, const Vector3& position, uint32_t count);
	void Emit(const std::string& name, const Vector3& position, const ParticleParameters& params, uint32_t count);

	void SetBlendMode(const std::string& name, BlendMode mode);

	void AddAccelerationField(const std::string& name, const Vector3& acceleration, const AABB& area);

private:
	ParticleManager() = default;
	ParticleManager(const ParticleManager&) = delete;
	ParticleManager& operator=(const ParticleManager&) = delete;

	static std::unique_ptr<ParticleManager> instance_;

	DirectXCommon* dxCommon_ = nullptr;
	SrvManager* srvManager_ = nullptr;

	std::unordered_map<std::string, ParticleGroup> particleGroups_;

	std::unordered_map<std::string, AccelerationField> accelerationFields_;

	struct VertexData {
		Vector4 position;
		Vector2 texcoord;
		Vector3 normal;
	};
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexBuffer_;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_{};

	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;
	std::array<Microsoft::WRL::ComPtr<ID3D12PipelineState>, static_cast<size_t>(BlendMode::kCountOf)> graphicsPipelineStates_;

	std::mt19937 randomEngine_;

	const uint32_t kMaxInstanceCount_ = 1024;

private: 
	void CreateRootSignature();
	void CreateGraphicsPipeline();
	void CreateModel(); 
};
