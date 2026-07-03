#pragma once
#include <d3d12.h>
#include <wrl/client.h>
#include <cstdint>
#include <memory>
#include <vector>
#include <string>

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
	kRandom,        // ランダム
	kCountOfPostEffects, // エフェクトの種類
};

struct PostEffectParams {
	int32_t kernelSize;
	float intensity;
	float dirX;
	float dirY;
	float time;
	float padding[3];
};

/// <summary>
/// フルスクリーンポストエフェクトを管理するシングルトンクラス
/// </summary>
class PostEffect {
public:
	/// <summary>
	/// シングルトンインスタンスの取得
	/// </summary>
	/// <returns>インスタンスポインタ</returns>
	static PostEffect* GetInstance();

	/// <summary>
	/// デフォルトコンストラクタ (std::make_unique対応のためpublic)
	/// </summary>
	PostEffect() = default;

	/// <summary>
	/// 初期化（PSO 生成、SRV 登録）
	/// </summary>
	void Initialize(DirectXCommon* dxCommon, SrvManager* srvManager);

	/// <summary>
	/// 終了処理
	/// </summary>
	void Finalize();

	/// <summary>
	/// レンダーテクスチャから swap chain への fullscreen 描画を実行する
	/// PostDraw 内で CopyResource の代わりに呼ぶ
	/// </summary>
	void Draw(ID3D12Resource* renderTextureResource, uint32_t renderTextureSrvIndex);

	/// 現在のエフェクトを設定
	void SetEffectType(PostEffectType type) { currentEffect_ = type; }

	/// 現在のエフェクトを取得
	PostEffectType GetEffectType() const { return currentEffect_; }

	void SetKernelSize(int32_t size) { kernelSize_ = size; }
	int32_t GetKernelSize() const { return kernelSize_; }

	void SetIntensity(float intensity) { intensity_ = intensity; }
	float GetIntensity() const { return intensity_; }

	void SetDirX(float dirX) { dirX_ = dirX; }
	float GetDirX() const { return dirX_; }

	void SetDirY(float dirY) { dirY_ = dirY; }
	float GetDirY() const { return dirY_; }

	void SetDissolveThreshold(float t) { dissolveThreshold_ = t; }
	float GetDissolveThreshold() const { return dissolveThreshold_; }

	void SetDissolveEdgeWidth(float w) { dissolveEdgeWidth_ = w; }
	float GetDissolveEdgeWidth() const { return dissolveEdgeWidth_; }

	void SetDissolveMaskIndex(int idx) { dissolveMaskIndex_ = idx; }
	int GetDissolveMaskIndex() const { return dissolveMaskIndex_; }
	int GetDissolveMaskCount() const { return static_cast<int>(dissolveMaskSrvIndices_.size()); }



	~PostEffect() = default;

private:
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
	float dirX_ = 0.0f;
	float dirY_ = 0.0f;

	Microsoft::WRL::ComPtr<ID3D12RootSignature> dissolveRootSignature_;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> dissolvePSO_;
	std::vector<uint32_t> dissolveMaskSrvIndices_;
	float dissolveThreshold_ = 0.5f;
	float dissolveEdgeWidth_ = 0.05f;
	int dissolveMaskIndex_ = 0;

	float time_ = 0.0f;


	void CreateRootSignature();
	void CreateDissolveRootSignature();
	void CreateGraphicsPipelines();
	void LoadDissolveMasks();
};
