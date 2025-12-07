#pragma once
#include <d3d12.h>
#include <wrl/client.h>
namespace TDEngine {
	class DirectXCommon;
	class Camera;

	class Object3dCommon
	{
	public:
		void Initialize(DirectXCommon* dxCommon);

		//共通描画設定
		void SetupCommonState();

		DirectXCommon* GetDirectXCommon() { return dxCommon_; }

		//アクセッサ
		//セッター
		void SetDefaultCamera(Camera* camera) { defaultCamera_ = camera; }

		//ゲッター
		Camera* GetDefaultCamera() const { return defaultCamera_; }

	private:
		DirectXCommon* dxCommon_ = nullptr;
		Camera* defaultCamera_ = nullptr;
		Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;
		Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineState_;
		//ルートシグネチャの作成
		void CreateRootSignature(DirectXCommon* dxCommon);

		//グラフィックパイプラインの生成
		void CreateGraphicsPipeline(DirectXCommon* dxCommon);
	};
}