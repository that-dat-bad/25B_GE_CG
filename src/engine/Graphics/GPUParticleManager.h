#pragma once
#include <d3d12.h>
#include <wrl/client.h>
#include <string>
#include <memory>

#include "../base/Math/MyMath.h"
#include "DirectXCommon.h"
#include "SrvManager.h"
#include "BlendMode.h"

class GPUParticleManager {
public:
	// CPU側からサイズ計算に使うため定義
	struct ParticleCS {
		Vector3 translate;
		Vector3 scale;
		float lifeTime;
		Vector3 velocity;
		float currentTime;
		Vector4 color;
	};

	static GPUParticleManager* GetInstance();

	void Initialize(DirectXCommon* dxCommon, SrvManager* srvManager);
	void Finalize();

	void Update();
	// viewProjection と billboardMatrix を受け取って描画
	void Draw(const Matrix4x4& viewProjection, const Matrix4x4& billboardMatrix);

	void SetTexture(uint32_t srvIndex) { textureSrvIndex_ = srvIndex; }
	
	// Emit用のパラメータ設定
	void SetEmitParams(const Vector3& position, uint32_t count, float radius, float frequency);
	
	// 強制的に発生させるフラグ（毎フレーム呼び出すと発生し続ける）
	void Emit();

public:
	~GPUParticleManager() = default;

private:
	GPUParticleManager() = default;
	GPUParticleManager(const GPUParticleManager&) = delete;
	GPUParticleManager& operator=(const GPUParticleManager&) = delete;

	static std::unique_ptr<GPUParticleManager> instance_;

	DirectXCommon* dxCommon_ = nullptr;
	SrvManager* srvManager_ = nullptr;

	// Root Signatures and PSOs
	Microsoft::WRL::ComPtr<ID3D12RootSignature> computeRootSignature_;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> initPipelineState_;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> emitPipelineState_;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> updatePipelineState_;

	Microsoft::WRL::ComPtr<ID3D12RootSignature> graphicsRootSignature_;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineState_;

	// Buffers
	static const uint32_t kMaxParticles = 1024;
	Microsoft::WRL::ComPtr<ID3D12Resource> particleBuffer_;
	Microsoft::WRL::ComPtr<ID3D12Resource> freeListIndexBuffer_;
	Microsoft::WRL::ComPtr<ID3D12Resource> freeListBuffer_;

	uint32_t particleSrvIndex_ = 0;
	uint32_t particleUavIndex_ = 0;
	uint32_t freeListIndexUavIndex_ = 0;
	uint32_t freeListUavIndex_ = 0;

	// Constant Buffers
	Microsoft::WRL::ComPtr<ID3D12Resource> emitterSphereBuffer_;
	struct EmitterSphere {
		Vector3 translate;
		float radius;
		uint32_t count;
		float frequency;
		float frequencyTime;
		uint32_t emit;
	};
	EmitterSphere* emitterSphereData_ = nullptr;

	Microsoft::WRL::ComPtr<ID3D12Resource> perFrameBuffer_;
	struct PerFrame {
		float time;
		float deltaTime;
	};
	PerFrame* perFrameData_ = nullptr;

	Microsoft::WRL::ComPtr<ID3D12Resource> perViewBuffer_;
	struct PerView {
		Matrix4x4 viewProjection;
		Matrix4x4 billboardMatrix;
	};
	PerView* perViewData_ = nullptr;

	// Drawing resources
	struct VertexData {
		Vector4 position;
		Vector2 texcoord;
		Vector3 normal;
	};
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexBuffer_;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_{};

	uint32_t textureSrvIndex_ = 0;
	float time_ = 0.0f;

	void CreateComputePipeline();
	void CreateGraphicsPipeline();
	void CreateBuffers();
	void CreateModel();
	void InitializeParticles();
};
