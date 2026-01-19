#pragma once
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>
#include"../../engine/base/WinApp.h"
#include <array>
#include <dxcapi.h>
#pragma comment(lib, "dxcompiler.lib")

#include "../../../external/DirectXTex/DirectXTex.h"
#include<string>
#include<chrono>

class DirectXCommon
{
public:
	static DirectXCommon* GetInstance();

	static const uint32_t kRtvHeapDescriptorNum_ = 2; // ダブルバッファ用
	static const uint32_t kDsvHeapDescriptorNum_ = 1; // 深度バッファ用
	static const uint32_t kSwapChainBufferCount_ = 2;
	//static const uint32_t kMaxSRVCount_;//最大SRV数(最大テクスチャ枚数)


	// 初期化
	void Initialize(WinApp* winApp);

	//描画前処理
	void PreDraw();

	//描画後処理
	void PostDraw();

	Microsoft::WRL::ComPtr<IDxcBlob> CompileShader(const std::wstring& filePath, const wchar_t* profile);




	//--ゲッター--
	ID3D12Device* GetDevice() { return device_.Get(); }
	ID3D12GraphicsCommandList* GetCommandList() { return commandList_.Get(); }
	ID3D12CommandQueue* GetCommandQueue() { return commandQueue_.Get(); }
	ID3D12CommandAllocator* GetCommandAllocator() { return commandAllocator_.Get(); }
	IDXGISwapChain4* GetSwapChain() { return swapChain_.Get(); }
	ID3D12Resource* GetCurrentBackBuffer() {
		return swapChainResources_[swapChain_->GetCurrentBackBufferIndex()].Get();
	}
	D3D12_CPU_DESCRIPTOR_HANDLE GetCurrentRTVHandle() {
		return rtvHandles_[swapChain_->GetCurrentBackBufferIndex()];
	}
	ID3D12DescriptorHeap* GetRTVDescriptorHeap() { return rtvDescriptorHeap_.Get(); }
	ID3D12DescriptorHeap* GetDSVDescriptorHeap() { return dsvDescriptorHeap_.Get(); }
	ID3D12Resource* GetDepthStencilBuffer() { return depthStencilResource_.Get(); }
	IDxcUtils* GetDxcUtils() { return dxcUtils_.Get(); }
	IDxcCompiler3* GetDxcCompiler() { return dxcCompiler_.Get(); }
	IDxcIncludeHandler* GetIncludeHandler() { return includeHandler_.Get(); }
	DXGI_FORMAT GetRTVFormat() { return rtvFormat_; }
	UINT64 GetFenceValue() { return fenceValue_; }
	HANDLE GetFenceEvent() { return fenceEvent_; }
	ID3D12Fence* GetFence() { return fence_.Get(); }
	D3D12_RECT GetScissorRect() { return scissorRect_; }


	//セッター
	void IncrementFenceValue() { fenceValue_++; }





	Microsoft::WRL::ComPtr<ID3D12Resource> CreateBufferResource(size_t sizeInBytes);
	static D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(ID3D12DescriptorHeap* descriptorHeap, uint32_t descriptorSize, uint32_t index);
	static D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle(ID3D12DescriptorHeap* descriptorHeap, uint32_t descriptorSize, uint32_t index);
	Microsoft::WRL::ComPtr<ID3D12Resource> CreateTextureResource(ID3D12Device* device, const DirectX::TexMetadata& metadata);
	Microsoft::WRL::ComPtr<ID3D12Resource> UploadTextureData(ID3D12Resource* texture, const DirectX::ScratchImage& mipImages);
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE heapType, UINT numDescriptors, bool shaderVisible);

private:
	DirectXCommon() = default;
	~DirectXCommon() = default;
	DirectXCommon(const DirectXCommon&) = delete;
	DirectXCommon& operator=(const DirectXCommon&) = delete;



	Microsoft::WRL::ComPtr<IDXGIFactory7> dxgiFactory_ = nullptr;
	Microsoft::WRL::ComPtr<IDXGIAdapter4> useAdapter_ = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Device> device_ = nullptr;
	Microsoft::WRL::ComPtr<IDXGISwapChain4> swapChain_ = nullptr;
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator_ = nullptr;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList_ = nullptr;
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue_ = nullptr;

	// デスクリプタヒープ
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvDescriptorHeap_;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> dsvDescriptorHeap_;


	Microsoft::WRL::ComPtr<ID3D12Resource> depthStencilResource_;
	Microsoft::WRL::ComPtr<ID3D12Fence> fence_;

	// デスクリプタサイズ
	uint32_t rtvDescriptorSize_;
	//uint32_t srvDescriptorSize_;
	uint32_t dsvDescriptorSize_;

	D3D12_RECT scissorRect_{};

	Microsoft::WRL::ComPtr<IDxcUtils> dxcUtils_;
	Microsoft::WRL::ComPtr<IDxcCompiler3> dxcCompiler_ = nullptr;
	Microsoft::WRL::ComPtr<IDxcIncludeHandler> includeHandler_ = nullptr;


	std::array<Microsoft::WRL::ComPtr<ID3D12Resource>, 2> swapChainResources_;
	std::array<D3D12_CPU_DESCRIPTOR_HANDLE, 2> rtvHandles_;

	DXGI_FORMAT rtvFormat_;

	HANDLE fenceEvent_ = nullptr;
	UINT64 fenceValue_ = 0;

	D3D12_VIEWPORT viewport_{};


	//記録時間(FPS固定用	)
	std::chrono::steady_clock::time_point reference_;

	WinApp* winApp_ = nullptr;

	//デバイスの初期化
	void CreateDevice();

	//コマンド関連の初期化
	void CreateCommand();

	//スワップチェーンの初期化
	void CreateSwapChain();

	//深度バッファの生成
	void CreateDepthStencilBuffer();

	//深度ステンシルビューの生成
	void CreateDepthStencilView();

	//各種デスクリプタヒープの生成
	void CreateDescriptorHeaps();

	//レンダーターゲットビューの初期化
	void CreateRenderTargetView();

	//フェンスの作成
	void CreateFence();

	//ビューポート矩形の初期化
	void SetViewport();

	//シザー矩形の初期化
	void SetScissorRect();

	//DXCコンパイラの初期化
	void InitializeDxcCompiler();

	//FPS固定初期化
	void InitializeFixFPS();

	//FPS固定更新
	void UpdateFixFPS();
};