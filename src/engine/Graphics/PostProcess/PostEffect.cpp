#include "PostEffect.h"
#include "../System/DirectXCommon.h"
#include "../System/SrvManager.h"
#include "../System/TextureManager.h"
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
	LoadDissolveMasks();
	CreateGraphicsPipelines();

	size_t cbAlignedSize = (sizeof(PostEffectParams) + 0xff) & ~0xff;
	postEffectParamsBuffer_ = dxCommon_->CreateBufferResource(cbAlignedSize * 10);
	HRESULT hr = postEffectParamsBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&mappedPostEffectParams_));
	assert(SUCCEEDED(hr));
}

void PostEffect::Finalize() {
	instance_.reset();
}

void PostEffect::CreateRootSignature() {
	// ルートシグネチャ: t0 にテクスチャ、s0 にサンプラー
	D3D12_DESCRIPTOR_RANGE descriptorRange[1] = {};
	descriptorRange[0].BaseShaderRegister = 0;
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
	rootParameters[1].Descriptor.ShaderRegister = 0;

	D3D12_STATIC_SAMPLER_DESC staticSamplers[1] = {};
	staticSamplers[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	staticSamplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	staticSamplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	staticSamplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	staticSamplers[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	staticSamplers[0].MaxLOD = D3D12_FLOAT32_MAX;
	staticSamplers[0].ShaderRegister = 0;
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
	D3D12_DESCRIPTOR_RANGE descriptorRange0[1] = {};
	descriptorRange0[0].BaseShaderRegister = 0;
	descriptorRange0[0].NumDescriptors = 1;
	descriptorRange0[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descriptorRange0[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	D3D12_DESCRIPTOR_RANGE descriptorRange1[1] = {};
	descriptorRange1[0].BaseShaderRegister = 1;
	descriptorRange1[0].NumDescriptors = 1;
	descriptorRange1[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descriptorRange1[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	D3D12_ROOT_PARAMETER rootParameters[3] = {};
	rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameters[0].DescriptorTable.pDescriptorRanges = descriptorRange0;
	rootParameters[0].DescriptorTable.NumDescriptorRanges = 1;
	rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameters[1].Descriptor.ShaderRegister = 0;
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
	staticSamplers[0].ShaderRegister = 0;
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
	Microsoft::WRL::ComPtr<IDxcBlob> vsBlob = dxCommon_->CompileShader(L"./assets/shaders/FullScreen.VS.hlsl", L"vs_6_0");

	// ピクセルシェーダーのコンパイル（各種エフェクト）
	Microsoft::WRL::ComPtr<IDxcBlob> psNone = dxCommon_->CompileShader(L"./assets/shaders/CopyImage.PS.hlsl", L"ps_6_0");
	Microsoft::WRL::ComPtr<IDxcBlob> psGray = dxCommon_->CompileShader(L"./assets/shaders/GrayScale.PS.hlsl", L"ps_6_0");
	Microsoft::WRL::ComPtr<IDxcBlob> psVignette = dxCommon_->CompileShader(L"./assets/shaders/Vignette.PS.hlsl", L"ps_6_0");
	Microsoft::WRL::ComPtr<IDxcBlob> psBoxFilter = dxCommon_->CompileShader(L"./assets/shaders/BoxFilter.PS.hlsl", L"ps_6_0");
	Microsoft::WRL::ComPtr<IDxcBlob> psGaussBlur = dxCommon_->CompileShader(L"./assets/shaders/GaussBlur.PS.hlsl", L"ps_6_0");
	Microsoft::WRL::ComPtr<IDxcBlob> psKawaseBlur = dxCommon_->CompileShader(L"./assets/shaders/KawaseBlur.PS.hlsl", L"ps_6_0");
	Microsoft::WRL::ComPtr<IDxcBlob> psRadialBlur = dxCommon_->CompileShader(L"./assets/shaders/RadialBlur.PS.hlsl", L"ps_6_0");
	Microsoft::WRL::ComPtr<IDxcBlob> psDissolve = dxCommon_->CompileShader(L"./assets/shaders/Dissolve.PS.hlsl", L"ps_6_0");
	Microsoft::WRL::ComPtr<IDxcBlob> psRandom = dxCommon_->CompileShader(L"./assets/shaders/Random.PS.hlsl", L"ps_6_0");

	Microsoft::WRL::ComPtr<IDxcBlob> pixelShaders[static_cast<size_t>(PostEffectType::kCountOfPostEffects)];
	pixelShaders[static_cast<size_t>(PostEffectType::kNone)] = psNone;
	pixelShaders[static_cast<size_t>(PostEffectType::kGrayScale)] = psGray;
	pixelShaders[static_cast<size_t>(PostEffectType::kVignette)] = psVignette;
	pixelShaders[static_cast<size_t>(PostEffectType::kBoxFilter)] = psBoxFilter;
	pixelShaders[static_cast<size_t>(PostEffectType::kGaussBlur)] = psGaussBlur;
	pixelShaders[static_cast<size_t>(PostEffectType::kKawaseBlur)] = psKawaseBlur;
	pixelShaders[static_cast<size_t>(PostEffectType::kRadialBlur)] = psRadialBlur;
	pixelShaders[static_cast<size_t>(PostEffectType::kDissolve)] = psDissolve;
	pixelShaders[static_cast<size_t>(PostEffectType::kRandom)] = psRandom;

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

	// 各エフェクト用の PSO を生成 (Dissolveは別root signatureなのでスキップ)
	for (size_t i = 0; i < static_cast<size_t>(PostEffectType::kCountOfPostEffects); ++i) {
		if (i == static_cast<size_t>(PostEffectType::kDissolve)) { continue; }
		psoDesc.PS = { pixelShaders[i]->GetBufferPointer(), pixelShaders[i]->GetBufferSize() };
		HRESULT hr = dxCommon_->GetDevice()->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pipelineStates_[i]));
		assert(SUCCEEDED(hr));
	}

	psoDesc.pRootSignature = dissolveRootSignature_.Get();
	psoDesc.PS = { psDissolve->GetBufferPointer(), psDissolve->GetBufferSize() };
	HRESULT hrDissolve = dxCommon_->GetDevice()->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&dissolvePSO_));
	assert(SUCCEEDED(hrDissolve));
}

void PostEffect::Draw(ID3D12Resource* renderTextureResource, uint32_t renderTextureSrvIndex) {
	ID3D12GraphicsCommandList* commandList = dxCommon_->GetCommandList();

	time_ += 0.016f;

	size_t effectIndex = static_cast<size_t>(currentEffect_);
	if (currentEffect_ != PostEffectType::kDissolve) {
		commandList->SetGraphicsRootSignature(rootSignature_.Get());
		commandList->SetPipelineState(pipelineStates_[effectIndex].Get());
	}

	ID3D12DescriptorHeap* heaps[] = { srvManager_->descriptorHeap_.Get() };
	commandList->SetDescriptorHeaps(1, heaps);

	// ビューポートとシザー矩形を設定
	D3D12_VIEWPORT viewport = dxCommon_->GetViewport();
	commandList->RSSetViewports(1, &viewport);

	D3D12_RECT scissor = dxCommon_->GetScissorRect();
	commandList->RSSetScissorRects(1, &scissor);

	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	size_t cbAlignedSize = (sizeof(PostEffectParams) + 0xff) & ~0xff;

	if (currentEffect_ == PostEffectType::kGaussBlur) {
		// --- パス1: 横方向ブラー (renderTextures_[0] -> renderTextures_[1]) ---
		D3D12_RESOURCE_BARRIER barrierRT = {};
		barrierRT.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrierRT.Transition.pResource = dxCommon_->GetRenderTexture(1);
		barrierRT.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		barrierRT.Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
		barrierRT.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
		commandList->ResourceBarrier(1, &barrierRT);

		D3D12_CPU_DESCRIPTOR_HANDLE rtv1 = dxCommon_->GetRenderTextureRTVHandle(1);
		commandList->OMSetRenderTargets(1, &rtv1, false, nullptr);

		commandList->SetGraphicsRootDescriptorTable(0, srvManager_->GetGPUDescriptorHandle(renderTextureSrvIndex));

		PostEffectParams* param0 = reinterpret_cast<PostEffectParams*>(reinterpret_cast<uint8_t*>(mappedPostEffectParams_) + 0);
		param0->kernelSize = kernelSize_;
		param0->intensity = intensity_;
		param0->dirX = 1.0f;
		param0->dirY = 0.0f;
		commandList->SetGraphicsRootConstantBufferView(1, postEffectParamsBuffer_->GetGPUVirtualAddress());

		commandList->DrawInstanced(3, 1, 0, 0);

		// --- パス2: 縦方向ブラー (renderTextures_[1] -> backBuffer) ---
		D3D12_RESOURCE_BARRIER barrierSRV = {};
		barrierSRV.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrierSRV.Transition.pResource = dxCommon_->GetRenderTexture(1);
		barrierSRV.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		barrierSRV.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
		barrierSRV.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
		commandList->ResourceBarrier(1, &barrierSRV);

		D3D12_CPU_DESCRIPTOR_HANDLE destRtv = dxCommon_->GetCurrentRTVHandle();
		commandList->OMSetRenderTargets(1, &destRtv, false, nullptr);

		commandList->SetGraphicsRootDescriptorTable(0, srvManager_->GetGPUDescriptorHandle(dxCommon_->GetRenderTextureSrvIndex(1)));

		PostEffectParams* param1 = reinterpret_cast<PostEffectParams*>(reinterpret_cast<uint8_t*>(mappedPostEffectParams_) + cbAlignedSize);
		param1->kernelSize = kernelSize_;
		param1->intensity = intensity_;
		param1->dirX = 0.0f;
		param1->dirY = 1.0f;
		commandList->SetGraphicsRootConstantBufferView(1, postEffectParamsBuffer_->GetGPUVirtualAddress() + cbAlignedSize);

		commandList->DrawInstanced(3, 1, 0, 0);
	}
	else if (currentEffect_ == PostEffectType::kKawaseBlur) {
		int passes = kernelSize_;
		if (passes < 1) { passes = 1; }
		if (passes > 10) { passes = 10; }

		for (int i = 0; i < passes; ++i) {
			bool isLastPass = (i == passes - 1);
			int srcIndex = (i % 2 == 0) ? 0 : 1;
			int dstIndex = (i % 2 == 0) ? 1 : 0;

			if (!isLastPass) {
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

			uint32_t srvIndex = dxCommon_->GetRenderTextureSrvIndex(srcIndex);
			commandList->SetGraphicsRootDescriptorTable(0, srvManager_->GetGPUDescriptorHandle(srvIndex));

			size_t offset = cbAlignedSize * i;
			PostEffectParams* param = reinterpret_cast<PostEffectParams*>(reinterpret_cast<uint8_t*>(mappedPostEffectParams_) + offset);
			param->kernelSize = kernelSize_;
			param->intensity = (float)i + 0.5f;
			param->dirX = 0.0f;
			param->dirY = 0.0f;
			commandList->SetGraphicsRootConstantBufferView(1, postEffectParamsBuffer_->GetGPUVirtualAddress() + offset);

			commandList->DrawInstanced(3, 1, 0, 0);

			if (!isLastPass) {
				D3D12_RESOURCE_BARRIER barrierSRV = {};
				barrierSRV.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
				barrierSRV.Transition.pResource = dxCommon_->GetRenderTexture(dstIndex);
				barrierSRV.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
				barrierSRV.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
				barrierSRV.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
				commandList->ResourceBarrier(1, &barrierSRV);
			}
		}
	}
	else if (currentEffect_ == PostEffectType::kDissolve) {
		commandList->SetGraphicsRootSignature(dissolveRootSignature_.Get());
		commandList->SetPipelineState(dissolvePSO_.Get());

		commandList->SetGraphicsRootDescriptorTable(0, srvManager_->GetGPUDescriptorHandle(renderTextureSrvIndex));

		PostEffectParams* param0 = reinterpret_cast<PostEffectParams*>(reinterpret_cast<uint8_t*>(mappedPostEffectParams_) + 0);
		param0->kernelSize = 0;
		param0->intensity = dissolveThreshold_;
		param0->dirX = dissolveEdgeWidth_;
		param0->dirY = 0.0f;
		commandList->SetGraphicsRootConstantBufferView(1, postEffectParamsBuffer_->GetGPUVirtualAddress());

		int maskIdx = dissolveMaskIndex_;
		if (maskIdx < 0 || maskIdx >= static_cast<int>(dissolveMaskSrvIndices_.size())) { maskIdx = 0; }
		commandList->SetGraphicsRootDescriptorTable(2, srvManager_->GetGPUDescriptorHandle(dissolveMaskSrvIndices_[maskIdx]));

		commandList->DrawInstanced(3, 1, 0, 0);
	}
	else if (currentEffect_ == PostEffectType::kRadialBlur) {
		commandList->SetGraphicsRootDescriptorTable(0, srvManager_->GetGPUDescriptorHandle(renderTextureSrvIndex));

		PostEffectParams* param0 = reinterpret_cast<PostEffectParams*>(reinterpret_cast<uint8_t*>(mappedPostEffectParams_) + 0);
		param0->kernelSize = kernelSize_;
		param0->intensity = intensity_;
		param0->dirX = dirX_;
		param0->dirY = dirY_;
		commandList->SetGraphicsRootConstantBufferView(1, postEffectParamsBuffer_->GetGPUVirtualAddress());

		commandList->DrawInstanced(3, 1, 0, 0);
	}
	else {
		// 通常の1パス描画
		commandList->SetGraphicsRootDescriptorTable(0, srvManager_->GetGPUDescriptorHandle(renderTextureSrvIndex));

		PostEffectParams* param0 = reinterpret_cast<PostEffectParams*>(reinterpret_cast<uint8_t*>(mappedPostEffectParams_) + 0);
		param0->kernelSize = kernelSize_;
		param0->intensity = intensity_;
		param0->dirX = dirX_;
		param0->dirY = dirY_;
		param0->time = time_;
		commandList->SetGraphicsRootConstantBufferView(1, postEffectParamsBuffer_->GetGPUVirtualAddress());

		commandList->DrawInstanced(3, 1, 0, 0);
	}
}
