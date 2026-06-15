#include "PostEffect.h"
#include "../DirectXCommon.h"
#include "../SrvManager.h"
#include "../TextureManager.h"
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
	CreateDissolveRootSignature();
	CreateOutlineRootSignature();
	LoadDissolveMasks();
	CreateGraphicsPipelines();

	size_t cbAlignedSize = (sizeof(PostEffectParams) + 0xff) & ~0xff;
	postEffectParamsBuffer_ = dxCommon_->CreateBufferResource(cbAlignedSize * 256); // max 256 passes
	HRESULT hr = postEffectParamsBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&mappedPostEffectParams_));
	assert(SUCCEEDED(hr));
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

void PostEffect::CreateDissolveRootSignature() {
	// Dissolve用: t0(シーンテクスチャ) + t1(マスクテクスチャ) + b0(パラメータ)
	D3D12_DESCRIPTOR_RANGE descriptorRange0[1] = {};
	descriptorRange0[0].BaseShaderRegister = 0; // t0
	descriptorRange0[0].NumDescriptors = 1;
	descriptorRange0[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descriptorRange0[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	D3D12_DESCRIPTOR_RANGE descriptorRange1[1] = {};
	descriptorRange1[0].BaseShaderRegister = 1; // t1
	descriptorRange1[0].NumDescriptors = 1;
	descriptorRange1[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descriptorRange1[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	D3D12_ROOT_PARAMETER rootParameters[3] = {};
	// Root 0: t0 scene texture
	rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameters[0].DescriptorTable.pDescriptorRanges = descriptorRange0;
	rootParameters[0].DescriptorTable.NumDescriptorRanges = 1;
	// Root 1: b0 cbuffer
	rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameters[1].Descriptor.ShaderRegister = 0; // b0
	// Root 2: t1 mask texture
	rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameters[2].DescriptorTable.pDescriptorRanges = descriptorRange1;
	rootParameters[2].DescriptorTable.NumDescriptorRanges = 1;

	D3D12_STATIC_SAMPLER_DESC staticSamplers[1] = {};
	staticSamplers[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	staticSamplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	staticSamplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	staticSamplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	staticSamplers[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	staticSamplers[0].MaxLOD = D3D12_FLOAT32_MAX;
	staticSamplers[0].ShaderRegister = 0; // s0
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
		IID_PPV_ARGS(&dissolveRootSignature_));
	assert(SUCCEEDED(hr));
}

void PostEffect::CreateOutlineRootSignature() {
	// Outline用: t0(シーンテクスチャ) + t1(深度テクスチャ) + b0(パラメータ)
	D3D12_DESCRIPTOR_RANGE descriptorRange0[1] = {};
	descriptorRange0[0].BaseShaderRegister = 0; // t0
	descriptorRange0[0].NumDescriptors = 1;
	descriptorRange0[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descriptorRange0[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	D3D12_DESCRIPTOR_RANGE descriptorRange1[1] = {};
	descriptorRange1[0].BaseShaderRegister = 1; // t1
	descriptorRange1[0].NumDescriptors = 1;
	descriptorRange1[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descriptorRange1[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	D3D12_ROOT_PARAMETER rootParameters[3] = {};
	// Root 0: t0 scene texture
	rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameters[0].DescriptorTable.pDescriptorRanges = descriptorRange0;
	rootParameters[0].DescriptorTable.NumDescriptorRanges = 1;
	// Root 1: b0 cbuffer
	rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameters[1].Descriptor.ShaderRegister = 0; // b0
	// Root 2: t1 depth texture
	rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameters[2].DescriptorTable.pDescriptorRanges = descriptorRange1;
	rootParameters[2].DescriptorTable.NumDescriptorRanges = 1;

	D3D12_STATIC_SAMPLER_DESC staticSamplers[2] = {};
	staticSamplers[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	staticSamplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	staticSamplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	staticSamplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	staticSamplers[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	staticSamplers[0].MaxLOD = D3D12_FLOAT32_MAX;
	staticSamplers[0].ShaderRegister = 0; // s0
	staticSamplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	staticSamplers[1].Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
	staticSamplers[1].AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	staticSamplers[1].AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	staticSamplers[1].AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	staticSamplers[1].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	staticSamplers[1].MaxLOD = D3D12_FLOAT32_MAX;
	staticSamplers[1].ShaderRegister = 1; // s1
	staticSamplers[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

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
		IID_PPV_ARGS(&outlineRootSignature_));
	assert(SUCCEEDED(hr));
}

void PostEffect::LoadDissolveMasks() {
	// ノイズマスクテクスチャを読み込み
	const std::string maskPaths[] = {
		"assets/masks/noise0.png",
		"assets/masks/noise1.png",
	};
	for (const auto& path : maskPaths) {
		TextureManager::GetInstance()->LoadTexture(path);
		uint32_t srvIndex = TextureManager::GetInstance()->GetTextureIndexByFilePath(path);
		dissolveMaskSrvIndices_.push_back(srvIndex);
	}
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
	Microsoft::WRL::ComPtr<IDxcBlob> psKawaseBlur = dxCommon_->CompileShader(L"./assets/shaders/KawaseBlur.PS.hlsl", L"ps_6_0");
	Microsoft::WRL::ComPtr<IDxcBlob> psRadialBlur = dxCommon_->CompileShader(L"./assets/shaders/RadialBlur.PS.hlsl", L"ps_6_0");
	Microsoft::WRL::ComPtr<IDxcBlob> psDissolve = dxCommon_->CompileShader(L"./assets/shaders/Dissolve.PS.hlsl", L"ps_6_0");
	Microsoft::WRL::ComPtr<IDxcBlob> psLuminanceOutline = dxCommon_->CompileShader(L"./assets/shaders/LuminanceBasedOutline.PS.hlsl", L"ps_6_0");
	Microsoft::WRL::ComPtr<IDxcBlob> psDepthOutline = dxCommon_->CompileShader(L"./assets/shaders/DepthBasedOutline.PS.hlsl", L"ps_6_0");
	Microsoft::WRL::ComPtr<IDxcBlob> psRandom = dxCommon_->CompileShader(L"./assets/shaders/Random.PS.hlsl", L"ps_6_0");

	Microsoft::WRL::ComPtr<IDxcBlob> pixelShaders[static_cast<size_t>(PostEffectType::kCountOfPostEffects)];
	pixelShaders[static_cast<size_t>(PostEffectType::kNone)] = psNone;
	pixelShaders[static_cast<size_t>(PostEffectType::kGrayScale)] = psGray;
	pixelShaders[static_cast<size_t>(PostEffectType::kVignette)] = psVignette;
	pixelShaders[static_cast<size_t>(PostEffectType::kBoxFilter)] = psBoxFilter;
	pixelShaders[static_cast<size_t>(PostEffectType::kGaussBlur)] = psGaussBlur;
	pixelShaders[static_cast<size_t>(PostEffectType::kKawaseBlur)] = psKawaseBlur;
	pixelShaders[static_cast<size_t>(PostEffectType::kRadialBlur)] = psRadialBlur;
	pixelShaders[static_cast<size_t>(PostEffectType::kDissolve)] = psDissolve;  // placeholder for loop
	pixelShaders[static_cast<size_t>(PostEffectType::kLuminanceBasedOutline)] = psLuminanceOutline;
	pixelShaders[static_cast<size_t>(PostEffectType::kDepthBasedOutline)] = psDepthOutline;
	pixelShaders[static_cast<size_t>(PostEffectType::kRandom)] = psRandom;

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

	// 各エフェクト用の PSO を生成 (DissolveやOutlineなどは別root signatureを使用)
	for (size_t i = 0; i < static_cast<size_t>(PostEffectType::kCountOfPostEffects); ++i) {
		if (i == static_cast<size_t>(PostEffectType::kDissolve)) continue;
		if (i == static_cast<size_t>(PostEffectType::kLuminanceBasedOutline) || i == static_cast<size_t>(PostEffectType::kDepthBasedOutline)) continue;
		psoDesc.PS = { pixelShaders[i]->GetBufferPointer(), pixelShaders[i]->GetBufferSize() };
		HRESULT hr = dxCommon_->GetDevice()->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pipelineStates_[i]));
		assert(SUCCEEDED(hr));
	}

	// Dissolve PSO (別のルートシグネチャを使用)
	psoDesc.pRootSignature = dissolveRootSignature_.Get();
	psoDesc.PS = { psDissolve->GetBufferPointer(), psDissolve->GetBufferSize() };
	HRESULT hrDissolve = dxCommon_->GetDevice()->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&dissolvePSO_));
	assert(SUCCEEDED(hrDissolve));

	// Outline PSO
	psoDesc.pRootSignature = outlineRootSignature_.Get();
	psoDesc.PS = { psLuminanceOutline->GetBufferPointer(), psLuminanceOutline->GetBufferSize() };
	HRESULT hrLuminance = dxCommon_->GetDevice()->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pipelineStates_[static_cast<size_t>(PostEffectType::kLuminanceBasedOutline)]));
	assert(SUCCEEDED(hrLuminance));

	psoDesc.PS = { psDepthOutline->GetBufferPointer(), psDepthOutline->GetBufferSize() };
	HRESULT hrDepth = dxCommon_->GetDevice()->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pipelineStates_[static_cast<size_t>(PostEffectType::kDepthBasedOutline)]));
	assert(SUCCEEDED(hrDepth));
}

void PostEffect::Draw(ID3D12Resource* renderTextureResource, uint32_t renderTextureSrvIndex) {
	ID3D12GraphicsCommandList* commandList = dxCommon_->GetCommandList();
	time_ += 0.016f;

	// ビューポートとシザー矩形を設定
	D3D12_VIEWPORT viewport = dxCommon_->GetViewport();
	commandList->RSSetViewports(1, &viewport);

	D3D12_RECT scissor = dxCommon_->GetScissorRect();
	commandList->RSSetScissorRects(1, &scissor);

	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// SRV ヒープを設定
	ID3D12DescriptorHeap* heaps[] = { srvManager_->descriptorHeap_.Get() };
	commandList->SetDescriptorHeaps(1, heaps);

	if (activeEffects_.empty()) {
		ActivePostEffect noneEffect;
		noneEffect.type = PostEffectType::kNone;
		DrawSinglePass(commandList, noneEffect, 0, 0, true, 0);
		return;
	}

	uint32_t currentSrcIndex = 0; // renderTextures_[0] has the initial scene
	uint32_t passIndex = 0; // keeps track of the constant buffer offset

	for (size_t i = 0; i < activeEffects_.size(); ++i) {
		const ActivePostEffect& effect = activeEffects_[i];
		bool isLastEffect = (i == activeEffects_.size() - 1);
		uint32_t dstIndex = 1 - currentSrcIndex;

		if (effect.type == PostEffectType::kGaussBlur) {
			DrawSinglePass(commandList, effect, currentSrcIndex, dstIndex, false, passIndex++, 1.0f, 0.0f);
			DrawSinglePass(commandList, effect, dstIndex, currentSrcIndex, isLastEffect, passIndex++, 0.0f, 1.0f);
			if (!isLastEffect) currentSrcIndex = currentSrcIndex;
		} else if (effect.type == PostEffectType::kKawaseBlur) {
			int passes = effect.kernelSize;
			if (passes < 1) passes = 1;
			if (passes > 10) passes = 10;
			uint32_t tempSrc = currentSrcIndex;
			uint32_t tempDst = dstIndex;
			for (int j = 0; j < passes; ++j) {
				bool isFinalOfKawase = (j == passes - 1);
				float kawaseOffset = (float)j + 0.5f;
				DrawSinglePass(commandList, effect, tempSrc, tempDst, isLastEffect && isFinalOfKawase, passIndex++, 0.0f, 0.0f, kawaseOffset);
				if (!isLastEffect || !isFinalOfKawase) {
					std::swap(tempSrc, tempDst);
				}
			}
			if (!isLastEffect) currentSrcIndex = tempSrc;
		} else {
			DrawSinglePass(commandList, effect, currentSrcIndex, dstIndex, isLastEffect, passIndex++, 1.0f, 1.0f);
			if (!isLastEffect) currentSrcIndex = dstIndex;
		}
	}
}

void PostEffect::DrawSinglePass(ID3D12GraphicsCommandList* commandList, const ActivePostEffect& effect, uint32_t srcIndex, uint32_t dstIndex, bool isOutputToBackBuffer, uint32_t passIndex, float customDirX, float customDirY, float customIntensity) {
	size_t cbAlignedSize = (sizeof(PostEffectParams) + 0xff) & ~0xff;
	size_t effectIndex = static_cast<size_t>(effect.type);

	if (effect.type == PostEffectType::kDissolve) {
		commandList->SetGraphicsRootSignature(dissolveRootSignature_.Get());
		commandList->SetPipelineState(dissolvePSO_.Get());
	} else if (effect.type == PostEffectType::kLuminanceBasedOutline || effect.type == PostEffectType::kDepthBasedOutline) {
		commandList->SetGraphicsRootSignature(outlineRootSignature_.Get());
		commandList->SetPipelineState(pipelineStates_[effectIndex].Get());
	} else {
		commandList->SetGraphicsRootSignature(rootSignature_.Get());
		commandList->SetPipelineState(pipelineStates_[effectIndex].Get());
	}

	// Set Render Target
	if (!isOutputToBackBuffer) {
		D3D12_RESOURCE_BARRIER barrierRT = {};
		barrierRT.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrierRT.Transition.pResource = dxCommon_->GetRenderTexture(dstIndex);
		barrierRT.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		barrierRT.Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
		barrierRT.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
		commandList->ResourceBarrier(1, &barrierRT);

		D3D12_CPU_DESCRIPTOR_HANDLE rtv = dxCommon_->GetRenderTextureRTVHandle(dstIndex);
		commandList->OMSetRenderTargets(1, &rtv, false, nullptr);
	} else {
		D3D12_CPU_DESCRIPTOR_HANDLE destRtv = dxCommon_->GetCurrentRTVHandle();
		commandList->OMSetRenderTargets(1, &destRtv, false, nullptr);
	}

	// Wait, for depth outline we need the depth buffer to be readable.
	bool isDepth = (effect.type == PostEffectType::kDepthBasedOutline);
	if (isDepth) {
		D3D12_RESOURCE_BARRIER barrierDepth = {};
		barrierDepth.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrierDepth.Transition.pResource = dxCommon_->GetDepthStencilBuffer();
		barrierDepth.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		barrierDepth.Transition.StateBefore = D3D12_RESOURCE_STATE_DEPTH_WRITE;
		barrierDepth.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
		commandList->ResourceBarrier(1, &barrierDepth);
	}

	// Set SRV Input
	uint32_t srvIndex = dxCommon_->GetRenderTextureSrvIndex(srcIndex);
	commandList->SetGraphicsRootDescriptorTable(0, srvManager_->GetGPUDescriptorHandle(srvIndex));

	// Set Parameters
	size_t offset = cbAlignedSize * passIndex;
	PostEffectParams* param = reinterpret_cast<PostEffectParams*>(reinterpret_cast<uint8_t*>(mappedPostEffectParams_) + offset);
	param->projectionInverse = projectionInverse_;
	param->time = time_;

	if (effect.type == PostEffectType::kDissolve) {
		param->kernelSize = 0;
		param->intensity = effect.dissolveThreshold;
		param->dirX = effect.dissolveEdgeWidth;
		param->dirY = 0.0f;
		
		int maskIdx = effect.dissolveMaskIndex;
		if (maskIdx < 0 || maskIdx >= static_cast<int>(dissolveMaskSrvIndices_.size())) maskIdx = 0;
		commandList->SetGraphicsRootDescriptorTable(2, srvManager_->GetGPUDescriptorHandle(dissolveMaskSrvIndices_[maskIdx]));
	} else {
		param->kernelSize = effect.kernelSize;
		param->intensity = (customIntensity >= 0.0f) ? customIntensity : effect.intensity;
		param->dirX = customDirX;
		param->dirY = customDirY;
		
		if (isDepth) {
			commandList->SetGraphicsRootDescriptorTable(2, srvManager_->GetGPUDescriptorHandle(dxCommon_->GetDepthSrvIndex()));
		}
	}

	commandList->SetGraphicsRootConstantBufferView(1, postEffectParamsBuffer_->GetGPUVirtualAddress() + offset);
	commandList->DrawInstanced(3, 1, 0, 0);

	// Revert Render Target back to SRV if not last pass
	if (!isOutputToBackBuffer) {
		D3D12_RESOURCE_BARRIER barrierSRV = {};
		barrierSRV.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrierSRV.Transition.pResource = dxCommon_->GetRenderTexture(dstIndex);
		barrierSRV.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		barrierSRV.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
		barrierSRV.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
		commandList->ResourceBarrier(1, &barrierSRV);
	}

	if (isDepth) {
		D3D12_RESOURCE_BARRIER barrierDepth = {};
		barrierDepth.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrierDepth.Transition.pResource = dxCommon_->GetDepthStencilBuffer();
		barrierDepth.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		barrierDepth.Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
		barrierDepth.Transition.StateAfter = D3D12_RESOURCE_STATE_DEPTH_WRITE;
		commandList->ResourceBarrier(1, &barrierDepth);
	}
}
