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

// パーティクル1粒の情報（CPU側での計算用）
// パーティクル1粒の情報（CPU側での計算用）
struct Particle {
	Transform transform;
	Vector3 velocity;
	Vector3 acceleration; // 加速度
	Vector4 color;
	float lifeTime;
	float currentTime;
};

// パーティクル発生パラメータ
struct ParticleParameters {
	Vector3 minVelocity = { -1.0f, -1.0f, -1.0f };
	Vector3 maxVelocity = { 1.0f, 1.0f, 1.0f };
	Vector4 minColor = { 1.0f, 1.0f, 1.0f, 1.0f };
	Vector4 maxColor = { 1.0f, 1.0f, 1.0f, 1.0f };
	float minLifeTime = 1.0f;
	float maxLifeTime = 3.0f;
	Vector3 acceleration = { 0.0f, 0.0f, 0.0f }; // 重力など
};

// GPUに送るインスタンシングデータ (StructuredBuffer用)
struct ParticleInstancingData {
	Matrix4x4 WVP;
	Vector4 color;
};

// パーティクルグループ
struct ParticleGroup {
	// マテリアルデータ
	std::string textureFilePath;
	uint32_t textureSrvIndex;

	// パーティクルのリスト
	std::list<Particle> particles;

	// インスタンシングデータ用
	uint32_t instancingSrvIndex;
	Microsoft::WRL::ComPtr<ID3D12Resource> instancingResource;
	UINT instanceCount; // 描画するインスタンス数
	ParticleInstancingData* instancingDataPtr = nullptr; // 書き込み用ポインタ
	BlendMode blendMode = BlendMode::kNormal;
};

class ParticleManager
{
public: // シングルトンパターン
	static ParticleManager* GetInstance();

	void Initialize(DirectXCommon* dxCommon, SrvManager* srvManager);
	void Update();
	void Draw();
	void Finalize();

	// グループの作成
	void CreateParticleGroup(const std::string& name, const std::string& textureFilePath);

	// パーティクルの発生
	void Emit(const std::string& name, const Vector3& position, uint32_t count);
	void Emit(const std::string& name, const Vector3& position, const ParticleParameters& params, uint32_t count);

	// ブレンドモード設定
	void SetBlendMode(const std::string& name, BlendMode mode);

private:
	ParticleManager() = default;
	~ParticleManager() = default;
	ParticleManager(const ParticleManager&) = delete;
	ParticleManager& operator=(const ParticleManager&) = delete;

	static ParticleManager* instance_;

	// メンバ変数
	DirectXCommon* dxCommon_ = nullptr;
	SrvManager* srvManager_ = nullptr;

	// パーティクルグループコンテナ
	std::unordered_map<std::string, ParticleGroup> particleGroups_;

	// 共通リソース
	// 頂点データ（板ポリゴン）
	struct VertexData {
		Vector4 position;
		Vector2 texcoord;
		Vector3 normal;
	};
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexBuffer_;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_{};

	// パイプライン関連
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;
	std::array<Microsoft::WRL::ComPtr<ID3D12PipelineState>, static_cast<size_t>(BlendMode::kCountOf)> graphicsPipelineStates_;

	// ランダムエンジン
	std::mt19937 randomEngine_;

	// 最大インスタンス数（1グループあたり）
	const uint32_t kMaxInstanceCount_ = 1024;

private: // 内部関数
	void CreateRootSignature();
	void CreateGraphicsPipeline();
	void CreateModel(); // 板ポリゴンの作成
};