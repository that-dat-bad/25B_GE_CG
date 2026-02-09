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

#include <memory> 

class DirectXCommon
{
public:
	static DirectXCommon* GetInstance();

	static const uint32_t kRtvHeapDescriptorNum_ = 2; 
	static const uint32_t kDsvHeapDescriptorNum_ = 1; 
	static const uint32_t kSwapChainBufferCount_ = 2;


	void Initialize(WinApp* winApp);

	void PreDraw();

	void PostDraw();

	Microsoft::WRL::ComPtr<IDxcBlob> CompileShader(const std::wstring& filePath, const wchar_t* profile);




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


	void IncrementFenceValue() { fenceValue_++; }





	Microsoft::WRL::ComPtr<ID3D12Resource> CreateBufferResource(size_t sizeInBytes);
	static D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(ID3D12DescriptorHeap* descriptorHeap, uint32_t descriptorSize, uint32_t index);
	static D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle(ID3D12DescriptorHeap* descriptorHeap, uint32_t descriptorSize, uint32_t index);
	Microsoft::WRL::ComPtr<ID3D12Resource> CreateTextureResource(ID3D12Device* device, const DirectX::TexMetadata& metadata);
	Microsoft::WRL::ComPtr<ID3D12Resource> UploadTextureData(ID3D12Resource* texture, const DirectX::ScratchImage& mipImages);
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE heapType, UINT numDescriptors, bool shaderVisible);
	~DirectXCommon() = default;

private:
	DirectXCommon() = default;
	DirectXCommon(const DirectXCommon&) = delete;
	DirectXCommon& operator=(const DirectXCommon&) = delete;
	static std::unique_ptr<DirectXCommon> instance_;


	Microsoft::WRL::ComPtr<IDXGIFactory7> dxgiFactory_ = nullptr;
	Microsoft::WRL::ComPtr<IDXGIAdapter4> useAdapter_ = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Device> device_ = nullptr;
	Microsoft::WRL::ComPtr<IDXGISwapChain4> swapChain_ = nullptr;
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator_ = nullptr;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList_ = nullptr;
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue_ = nullptr;

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvDescriptorHeap_;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> dsvDescriptorHeap_;


	Microsoft::WRL::ComPtr<ID3D12Resource> depthStencilResource_;
	Microsoft::WRL::ComPtr<ID3D12Fence> fence_;

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


	std::chrono::steady_clock::time_point reference_;

	WinApp* winApp_ = nullptr;

	void CreateDevice();

	void CreateCommand();

	void CreateSwapChain();

	void CreateDepthStencilBuffer();

	void CreateDepthStencilView();

	void CreateDescriptorHeaps();

	void CreateRenderTargetView();

	void CreateFence();

	void SetViewport();

	void SetScissorRect();

	void InitializeDxcCompiler();

	void InitializeFixFPS();

	void UpdateFixFPS();
};
