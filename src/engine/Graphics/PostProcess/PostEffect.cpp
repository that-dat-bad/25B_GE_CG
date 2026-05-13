#include "PostEffect.h"
#include "../DirectXCommon.h"
#include "../SrvManager.h"
#include <cassert>

std::unique_ptr<PostEffect> PostEffect::instance_ = nullptr;

PostEffect* PostEffect::GetInstance() {
	if (instance_ == nullptr) {
		instance_.reset(new PostEffect());
	}
	return instance_.get();
}

void PostEffect::Initialize(DirectXCommon* dxCommon, SrvManager* srvManager) {
	dxCommon_ = dxCommon;
	srvManager_ = srvManager;

	CreateRootSignature();
	CreateGraphicsPipelines();

	postEffectParamsBuffer_ = dxCommon_->CreateBufferResource((sizeof(PostEffectParams) + 0xff) & ~0xff);
	HRESULT hr = postEffectParamsBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&mappedPostEffectParams_));
	assert(SUCCEEDED(hr));
	mappedPostEffectParams_->kernelSize = kernelSize_;
	mappedPostEffectParams_->intensity = intensity_;
}

void PostEffect::Finalize() {
	instance_.reset();
}

void PostEffect::CreateRootSignature() {
	// ルートシグネチャ: t0 にテクスチャ、s0 にサンプラー
	D3D12_DESCRIPTOR_RANGE descriptorRange[1] = {};
	descriptorRange[0].BaseShaderRegister = 0;                              // t0
	descriptorRange[0].NumDescriptors = 1;
	descriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	D3D12_ROOT_PARAMETER rootParameters[2] = {};
	rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameters[0].DescriptorTable.pDescriptorRanges = descriptorRange;
	rootParameters[0].DescriptorTable.NumDescriptorRanges = 1;

	rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameters[1].Descriptor.ShaderRegister = 0; // b0

	D3D12_STATIC_SAMPLER_DESC staticSamplers[1] = {};
	staticSamplers[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	staticSamplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	staticSamplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	staticSamplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	staticSamplers[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	staticSamplers[0].MaxLOD = D3D12_FLOAT32_MAX;
	staticSamplers[0].ShaderRegister = 0;                                   // s0
	staticSamplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	D3D12_ROOT_SIGNATURE_DESC desc{};
	desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	desc.pParameters = rootParameters;
	desc.NumParameters = _countof(rootParameters);
	desc.pStaticSamplers = staticSamplers;
	desc.NumStaticSamplers = _countof(staticSamplers);

	Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob;
	Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;
	HRESULT hr = D3D12SerializeRootSignature(&desc, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
	assert(SUCCEEDED(hr));

	hr = dxCommon_->GetDevice()->CreateRootSignature(
		0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(),
		IID_PPV_ARGS(&rootSignature_));
	assert(SUCCEEDED(hr));
}

void PostEffect::CreateGraphicsPipelines() {
	// 共通の頂点シェーダー（フルスクリーン三角形）
	Microsoft::WRL::ComPtr<IDxcBlob> vsBlob = dxCommon_->CompileShader(L"./assets/shaders/FullScreen.VS.hlsl", L"vs_6_0");

	// エフェクトごとのピクセルシェーダー
	Microsoft::WRL::ComPtr<IDxcBlob> psNone = dxCommon_->CompileShader(L"./assets/shaders/CopyImage.PS.hlsl", L"ps_6_0");
	Microsoft::WRL::ComPtr<IDxcBlob> psGray = dxCommon_->CompileShader(L"./assets/shaders/GrayScale.PS.hlsl", L"ps_6_0");
	Microsoft::WRL::ComPtr<IDxcBlob> psVignette = dxCommon_->CompileShader(L"./assets/shaders/Vignette.PS.hlsl", L"ps_6_0");
	Microsoft::WRL::ComPtr<IDxcBlob> psBoxFilter = dxCommon_->CompileShader(L"./assets/shaders/BoxFilter.PS.hlsl", L"ps_6_0");
	Microsoft::WRL::ComPtr<IDxcBlob> psGaussBlur = dxCommon_->CompileShader(L"./assets/shaders/GaussBlur.PS.hlsl", L"ps_6_0");

	Microsoft::WRL::ComPtr<IDxcBlob> pixelShaders[static_cast<size_t>(PostEffectType::kCountOfPostEffects)];
	pixelShaders[static_cast<size_t>(PostEffectType::kNone)] = psNone;
	pixelShaders[static_cast<size_t>(PostEffectType::kGrayScale)] = psGray;
	pixelShaders[static_cast<size_t>(PostEffectType::kVignette)] = psVignette;
	pixelShaders[static_cast<size_t>(PostEffectType::kBoxFilter)] = psBoxFilter;
	pixelShaders[static_cast<size_t>(PostEffectType::kGaussBlur)] = psGaussBlur;

	// PSO のベース設定
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc{};
	psoDesc.pRootSignature = rootSignature_.Get();

	// フルスクリーン三角形: 頂点入力なし（SV_VertexID で生成）
	psoDesc.InputLayout = { nullptr, 0 };

	psoDesc.VS = { vsBlob->GetBufferPointer(), vsBlob->GetBufferSize() };

	// ラスタライザ
	D3D12_RASTERIZER_DESC rasterizer{};
	rasterizer.CullMode = D3D12_CULL_MODE_NONE;
	rasterizer.FillMode = D3D12_FILL_MODE_SOLID;
	psoDesc.RasterizerState = rasterizer;

	// ブレンド（不透明）
	D3D12_BLEND_DESC blendDesc{};
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	psoDesc.BlendState = blendDesc;

	// 深度なし
	D3D12_DEPTH_STENCIL_DESC depthStencil{};
	depthStencil.DepthEnable = false;
	psoDesc.DepthStencilState = depthStencil;

	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = dxCommon_->GetRTVFormat();
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.SampleDesc.Count = 1;
	psoDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;

	// 各エフェクト用の PSO を生成
	for (size_t i = 0; i < static_cast<size_t>(PostEffectType::kCountOfPostEffects); ++i) {
		psoDesc.PS = { pixelShaders[i]->GetBufferPointer(), pixelShaders[i]->GetBufferSize() };
		HRESULT hr = dxCommon_->GetDevice()->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pipelineStates_[i]));
		assert(SUCCEEDED(hr));
	}
}

void PostEffect::Draw(ID3D12Resource* renderTextureResource, uint32_t renderTextureSrvIndex) {
	ID3D12GraphicsCommandList* commandList = dxCommon_->GetCommandList();

	// PSO とルートシグネチャをセット
	size_t effectIndex = static_cast<size_t>(currentEffect_);
	commandList->SetGraphicsRootSignature(rootSignature_.Get());
	commandList->SetPipelineState(pipelineStates_[effectIndex].Get());

	// SRV ヒープを設定（SrvManager が管理するヒープ）
	ID3D12DescriptorHeap* heaps[] = { srvManager_->descriptorHeap_.Get() };
	commandList->SetDescriptorHeaps(1, heaps);

	// t0 にレンダーテクスチャの SRV を設定
	commandList->SetGraphicsRootDescriptorTable(0, srvManager_->GetGPUDescriptorHandle(renderTextureSrvIndex));

	// b0 にポストエフェクト用定数バッファを設定
	mappedPostEffectParams_->kernelSize = kernelSize_;
	mappedPostEffectParams_->intensity = intensity_;
	commandList->SetGraphicsRootConstantBufferView(1, postEffectParamsBuffer_->GetGPUVirtualAddress());

	// ビューポートとシザー矩形を設定
	D3D12_VIEWPORT viewport = dxCommon_->GetViewport();
	commandList->RSSetViewports(1, &viewport);

	D3D12_RECT scissor = dxCommon_->GetScissorRect();
	commandList->RSSetScissorRects(1, &scissor);

	// フルスクリーン三角形を描画（頂点3つ、SV_VertexID で位置を生成）
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	commandList->DrawInstanced(3, 1, 0, 0);
}
