#pragma once
#include <d3d12.h>
#include <wrl/client.h>
#include <cstdint>
#include <memory>
#include <vector>
#include <string>
#include "../../base/Math/MyMath.h"

class DirectXCommon;
class SrvManager;

/// ポストエフェクトの種類
enum class PostEffectType : uint32_t {
	kNone = 0,      // エフェクトなし
	kGrayScale,     // グレースケール
	kVignette,      // ビネット
	kBoxFilter,     // ボックスフィルタ
	kGaussBlur,     // ガウスブラー
	kKawaseBlur,    // 川瀬式ブラー
	kRadialBlur,    // ラジアルブラー
	kDissolve,      // ディゾルブ
	kLuminanceBasedOutline, // 輝度ベースの輪郭線
	kDepthBasedOutline,     // 深度ベースの輪郭線
	kRandom,        // ランダム
	kCountOfPostEffects, // エフェクトの種類
};

struct PostEffectParams {
	int32_t kernelSize;
	float intensity;
	float dirX;
	float dirY;
	MyMath::Matrix4x4 projectionInverse;
	float time;
	float padding[3];
};

struct ActivePostEffect {
	PostEffectType type = PostEffectType::kNone;
	int32_t kernelSize = 3;
	float intensity = 1.0f;
	float dissolveThreshold = 0.5f;
	float dissolveEdgeWidth = 0.05f;
	int dissolveMaskIndex = 0;
};

/// フルスクリーンポストエフェクトを管理するシングルトンクラス
class PostEffect {
public:
	static PostEffect* GetInstance();

	/// 初期化（PSO 生成、SRV 登録）
	void Initialize(DirectXCommon* dxCommon, SrvManager* srvManager);

	/// 終了処理
	void Finalize();

	/// レンダーテクスチャから swap chain への fullscreen 描画を実行する
	/// PostDraw 内で CopyResource の代わりに呼ぶ
	void Draw(ID3D12Resource* renderTextureResource, uint32_t renderTextureSrvIndex);

	/// 現在のエフェクトのリストを取得・設定
	std::vector<ActivePostEffect>& GetActiveEffects() { return activeEffects_; }
	const std::vector<ActivePostEffect>& GetActiveEffects() const { return activeEffects_; }

	int GetDissolveMaskCount() const { return static_cast<int>(dissolveMaskSrvIndices_.size()); }

	void SetProjectionInverse(const MyMath::Matrix4x4& mat) { projectionInverse_ = mat; }
	const MyMath::Matrix4x4& GetProjectionInverse() const { return projectionInverse_; }

	~PostEffect() = default;

private:
	PostEffect() = default;
	PostEffect(const PostEffect&) = delete;
	PostEffect& operator=(const PostEffect&) = delete;

	static std::unique_ptr<PostEffect> instance_;

	DirectXCommon* dxCommon_ = nullptr;
	SrvManager* srvManager_ = nullptr;

	// ルートシグネチャ（全エフェクト共通）
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;

	// エフェクトごとの PSO
	Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineStates_[static_cast<size_t>(PostEffectType::kCountOfPostEffects)];

	std::vector<ActivePostEffect> activeEffects_;

	Microsoft::WRL::ComPtr<ID3D12Resource> postEffectParamsBuffer_;
	PostEffectParams* mappedPostEffectParams_ = nullptr;
	MyMath::Matrix4x4 projectionInverse_ = {};

	// Dissolve 用
	Microsoft::WRL::ComPtr<ID3D12RootSignature> dissolveRootSignature_;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> dissolvePSO_;
	std::vector<uint32_t> dissolveMaskSrvIndices_; // noise texture SRV indices

	float time_ = 0.0f;

	void DrawSinglePass(ID3D12GraphicsCommandList* commandList, const ActivePostEffect& effect, uint32_t srcIndex, uint32_t dstIndex, bool isOutputToBackBuffer, uint32_t passIndex, float customDirX = 1.0f, float customDirY = 1.0f, float customIntensity = -1.0f);

	void CreateRootSignature();
	void CreateDissolveRootSignature();
	void CreateGraphicsPipelines();
	void LoadDissolveMasks();

	Microsoft::WRL::ComPtr<ID3D12RootSignature> outlineRootSignature_;
	void CreateOutlineRootSignature();
};
