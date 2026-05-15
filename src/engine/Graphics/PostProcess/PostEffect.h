#pragma once
#include <d3d12.h>
#include <wrl/client.h>
#include <cstdint>
#include <memory>

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
	kCountOfPostEffects, // エフェクトの種類
};

struct PostEffectParams {
	int32_t kernelSize;
	float intensity;
	float dirX;
	float dirY;
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

	/// 現在のエフェクトを設定
	void SetEffectType(PostEffectType type) { currentEffect_ = type; }

	/// 現在のエフェクトを取得
	PostEffectType GetEffectType() const { return currentEffect_; }

	void SetKernelSize(int32_t size) { kernelSize_ = size; }
	int32_t GetKernelSize() const { return kernelSize_; }

	void SetIntensity(float intensity) { intensity_ = intensity; }
	float GetIntensity() const { return intensity_; }

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

	// 現在のエフェクト種類
	PostEffectType currentEffect_ = PostEffectType::kNone;

	Microsoft::WRL::ComPtr<ID3D12Resource> postEffectParamsBuffer_;
	PostEffectParams* mappedPostEffectParams_ = nullptr;
	int32_t kernelSize_ = 3;
	float intensity_ = 1.0f;

	void CreateRootSignature();
	void CreateGraphicsPipelines();
};
