#include "GPUParticleManager.h"
#include <cassert>
#include <vector>

std::unique_ptr<GPUParticleManager> GPUParticleManager::instance_ = nullptr;

GPUParticleManager* GPUParticleManager::GetInstance() {
    if (!instance_) {
        instance_.reset(new GPUParticleManager());
    }
    return instance_.get();
}

void GPUParticleManager::Initialize(DirectXCommon* dxCommon, SrvManager* srvManager) {
    dxCommon_ = dxCommon;
    srvManager_ = srvManager;

    CreateBuffers();
    CreateComputePipeline();
    CreateGraphicsPipeline();
    CreateModel();

    InitializeParticles();
}

void GPUParticleManager::Finalize() {
    // Release resources
    particleBuffer_.Reset();
    freeListIndexBuffer_.Reset();
    freeListBuffer_.Reset();
    emitterSphereBuffer_.Reset();
    perFrameBuffer_.Reset();
    perViewBuffer_.Reset();
    vertexBuffer_.Reset();

    computeRootSignature_.Reset();
    initPipelineState_.Reset();
    emitPipelineState_.Reset();
    updatePipelineState_.Reset();

    graphicsRootSignature_.Reset();
    graphicsPipelineState_.Reset();
}

void GPUParticleManager::CreateBuffers() {
    auto device = dxCommon_->GetDevice();

    // Allocate indices first to ensure UAVs are contiguous
    particleUavIndex_ = srvManager_->Allocate();
    freeListIndexUavIndex_ = srvManager_->Allocate();
    freeListUavIndex_ = srvManager_->Allocate();
    
    particleSrvIndex_ = srvManager_->Allocate();

    // 1. Particle Buffer (UAV/SRV)
    particleBuffer_ = dxCommon_->CreateUAVBufferResource(sizeof(ParticleCS) * kMaxParticles);
    srvManager_->CreateUAVforStructuredBuffer(particleUavIndex_, particleBuffer_.Get(), kMaxParticles, sizeof(ParticleCS));
    srvManager_->CreateSRVforStructuredBuffer(particleSrvIndex_, particleBuffer_.Get(), kMaxParticles, sizeof(ParticleCS));

    // 2. FreeListIndex Buffer (UAV)
    freeListIndexBuffer_ = dxCommon_->CreateUAVBufferResource(sizeof(int32_t));
    srvManager_->CreateUAVforStructuredBuffer(freeListIndexUavIndex_, freeListIndexBuffer_.Get(), 1, sizeof(int32_t));

    // 3. FreeList Buffer (UAV)
    freeListBuffer_ = dxCommon_->CreateUAVBufferResource(sizeof(uint32_t) * kMaxParticles);
    srvManager_->CreateUAVforStructuredBuffer(freeListUavIndex_, freeListBuffer_.Get(), kMaxParticles, sizeof(uint32_t));

    // 4. EmitterSphere (Constant Buffer)
    emitterSphereBuffer_ = dxCommon_->CreateBufferResource(sizeof(EmitterSphere));
    emitterSphereBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&emitterSphereData_));
    emitterSphereData_->translate = { 0, 0, 0 };
    emitterSphereData_->radius = 1.0f;
    emitterSphereData_->count = 10;
    emitterSphereData_->frequency = 0.5f;
    emitterSphereData_->frequencyTime = 0.0f;
    emitterSphereData_->emit = 0;

    // 5. PerFrame (Constant Buffer)
    perFrameBuffer_ = dxCommon_->CreateBufferResource(sizeof(PerFrame));
    perFrameBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&perFrameData_));
    perFrameData_->time = 0.0f;
    perFrameData_->deltaTime = 1.0f / 60.0f;

    // 6. PerView (Constant Buffer)
    perViewBuffer_ = dxCommon_->CreateBufferResource(sizeof(PerView));
    perViewBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&perViewData_));
}

void GPUParticleManager::CreateComputePipeline() {
    auto device = dxCommon_->GetDevice();

    // Root Signature
    D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
    descriptionRootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_NONE;

    D3D12_ROOT_PARAMETER rootParameters[3] = {};
    
    // Param 0: Descriptor Table for UAVs
    D3D12_DESCRIPTOR_RANGE descriptorRange[1] = {};
    descriptorRange[0].BaseShaderRegister = 0; // u0
    descriptorRange[0].NumDescriptors = 3;     // u0, u1, u2
    descriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
    descriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    rootParameters[0].DescriptorTable.pDescriptorRanges = descriptorRange;
    rootParameters[0].DescriptorTable.NumDescriptorRanges = 1;

    // Param 1: CBV for Emitter (b0)
    rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    rootParameters[1].Descriptor.ShaderRegister = 0; // b0

    // Param 2: CBV for PerFrame (b1)
    rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    rootParameters[2].Descriptor.ShaderRegister = 1; // b1

    descriptionRootSignature.pParameters = rootParameters;
    descriptionRootSignature.NumParameters = 3;

    Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob;
    Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;
    HRESULT hr = D3D12SerializeRootSignature(&descriptionRootSignature, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
    if (FAILED(hr)) {
        OutputDebugStringA((char*)errorBlob->GetBufferPointer());
        assert(false);
    }
    hr = device->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&computeRootSignature_));
    assert(SUCCEEDED(hr));

    // Compile Shaders
    Microsoft::WRL::ComPtr<IDxcBlob> initCS = dxCommon_->CompileShader(L"assets/shaders/InitializeParticle.CS.hlsl", L"cs_6_0");
    Microsoft::WRL::ComPtr<IDxcBlob> emitCS = dxCommon_->CompileShader(L"assets/shaders/EmitParticle.CS.hlsl", L"cs_6_0");
    Microsoft::WRL::ComPtr<IDxcBlob> updateCS = dxCommon_->CompileShader(L"assets/shaders/UpdateParticle.CS.hlsl", L"cs_6_0");

    // Pipeline States
    D3D12_COMPUTE_PIPELINE_STATE_DESC psoDesc{};
    psoDesc.pRootSignature = computeRootSignature_.Get();
    
    // Init
    psoDesc.CS = { initCS->GetBufferPointer(), initCS->GetBufferSize() };
    hr = device->CreateComputePipelineState(&psoDesc, IID_PPV_ARGS(&initPipelineState_));
    assert(SUCCEEDED(hr));

    // Emit
    psoDesc.CS = { emitCS->GetBufferPointer(), emitCS->GetBufferSize() };
    hr = device->CreateComputePipelineState(&psoDesc, IID_PPV_ARGS(&emitPipelineState_));
    assert(SUCCEEDED(hr));

    // Update
    psoDesc.CS = { updateCS->GetBufferPointer(), updateCS->GetBufferSize() };
    hr = device->CreateComputePipelineState(&psoDesc, IID_PPV_ARGS(&updatePipelineState_));
    assert(SUCCEEDED(hr));
}

void GPUParticleManager::CreateGraphicsPipeline() {
    auto device = dxCommon_->GetDevice();

    // Root Signature
    D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
    descriptionRootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    D3D12_ROOT_PARAMETER rootParameters[3] = {};
    
    // SRV for Particles (t0)
    D3D12_DESCRIPTOR_RANGE descriptorRangeSRV[1] = {};
    descriptorRangeSRV[0].BaseShaderRegister = 0;
    descriptorRangeSRV[0].NumDescriptors = 1;
    descriptorRangeSRV[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    descriptorRangeSRV[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
    rootParameters[0].DescriptorTable.pDescriptorRanges = descriptorRangeSRV;
    rootParameters[0].DescriptorTable.NumDescriptorRanges = 1;

    // CBV for PerView (b0)
    rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
    rootParameters[1].Descriptor.ShaderRegister = 0;

    // SRV for Texture (t0 in PS)
    D3D12_DESCRIPTOR_RANGE descriptorRangeTex[1] = {};
    descriptorRangeTex[0].BaseShaderRegister = 0;
    descriptorRangeTex[0].NumDescriptors = 1;
    descriptorRangeTex[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    descriptorRangeTex[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
    rootParameters[2].DescriptorTable.pDescriptorRanges = descriptorRangeTex;
    rootParameters[2].DescriptorTable.NumDescriptorRanges = 1;

    D3D12_STATIC_SAMPLER_DESC staticSamplers[1] = {};
    staticSamplers[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
    staticSamplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    staticSamplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    staticSamplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    staticSamplers[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
    staticSamplers[0].MaxLOD = D3D12_FLOAT32_MAX;
    staticSamplers[0].ShaderRegister = 0;
    staticSamplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

    descriptionRootSignature.pParameters = rootParameters;
    descriptionRootSignature.NumParameters = 3;
    descriptionRootSignature.pStaticSamplers = staticSamplers;
    descriptionRootSignature.NumStaticSamplers = 1;

    Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob;
    Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;
    HRESULT hr = D3D12SerializeRootSignature(&descriptionRootSignature, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
    if (FAILED(hr)) {
        OutputDebugStringA((char*)errorBlob->GetBufferPointer());
        assert(false);
    }
    hr = device->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&graphicsRootSignature_));
    assert(SUCCEEDED(hr));

    // Input Layout
    D3D12_INPUT_ELEMENT_DESC inputElementDescs[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,       0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    };

    // Compile Shaders
    Microsoft::WRL::ComPtr<IDxcBlob> vsBlob = dxCommon_->CompileShader(L"assets/shaders/GPUParticle.VS.hlsl", L"vs_6_0");
    Microsoft::WRL::ComPtr<IDxcBlob> psBlob = dxCommon_->CompileShader(L"assets/shaders/GPUParticle.PS.hlsl", L"ps_6_0");

    D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc{};
    graphicsPipelineStateDesc.pRootSignature = graphicsRootSignature_.Get();
    graphicsPipelineStateDesc.InputLayout = { inputElementDescs, _countof(inputElementDescs) };
    graphicsPipelineStateDesc.VS = { vsBlob->GetBufferPointer(), vsBlob->GetBufferSize() };
    graphicsPipelineStateDesc.PS = { psBlob->GetBufferPointer(), psBlob->GetBufferSize() };
    graphicsPipelineStateDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
    graphicsPipelineStateDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
    
    // Additive Blending
    D3D12_RENDER_TARGET_BLEND_DESC blendDesc{};
    blendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
    blendDesc.BlendEnable = TRUE;
    blendDesc.SrcBlend = D3D12_BLEND_SRC_ALPHA;
    blendDesc.DestBlend = D3D12_BLEND_ONE;
    blendDesc.BlendOp = D3D12_BLEND_OP_ADD;
    blendDesc.SrcBlendAlpha = D3D12_BLEND_ONE;
    blendDesc.DestBlendAlpha = D3D12_BLEND_ZERO;
    blendDesc.BlendOpAlpha = D3D12_BLEND_OP_ADD;

    graphicsPipelineStateDesc.BlendState.RenderTarget[0] = blendDesc;
    graphicsPipelineStateDesc.DepthStencilState.DepthEnable = TRUE;
    graphicsPipelineStateDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO; // Disable depth write for particles
    graphicsPipelineStateDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
    graphicsPipelineStateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
    graphicsPipelineStateDesc.NumRenderTargets = 1;
    graphicsPipelineStateDesc.RTVFormats[0] = dxCommon_->GetRTVFormat();
    graphicsPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    graphicsPipelineStateDesc.SampleDesc.Count = 1;

    hr = device->CreateGraphicsPipelineState(&graphicsPipelineStateDesc, IID_PPV_ARGS(&graphicsPipelineState_));
    assert(SUCCEEDED(hr));
}

void GPUParticleManager::CreateModel() {
    vertexBuffer_ = dxCommon_->CreateBufferResource(sizeof(VertexData) * 4);
    
    vertexBufferView_.BufferLocation = vertexBuffer_->GetGPUVirtualAddress();
    vertexBufferView_.SizeInBytes = sizeof(VertexData) * 4;
    vertexBufferView_.StrideInBytes = sizeof(VertexData);
    
    VertexData* vertexData = nullptr;
    vertexBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
    
    // Quad (Triangle Strip)
    vertexData[0] = { {-0.5f, -0.5f, 0.0f, 1.0f}, {0.0f, 1.0f}, {0.0f, 0.0f, -1.0f} }; // Bottom Left
    vertexData[1] = { {-0.5f,  0.5f, 0.0f, 1.0f}, {0.0f, 0.0f}, {0.0f, 0.0f, -1.0f} }; // Top Left
    vertexData[2] = { { 0.5f, -0.5f, 0.0f, 1.0f}, {1.0f, 1.0f}, {0.0f, 0.0f, -1.0f} }; // Bottom Right
    vertexData[3] = { { 0.5f,  0.5f, 0.0f, 1.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, -1.0f} }; // Top Right
}

void GPUParticleManager::InitializeParticles() {
    auto commandList = dxCommon_->GetCommandList();
    
    commandList->SetPipelineState(initPipelineState_.Get());
    commandList->SetComputeRootSignature(computeRootSignature_.Get());
    
    // Bind UAVs (particleUavIndex_ is the start of the contiguous descriptor table for u0, u1, u2)
    srvManager_->SetGraphicsRootDescriptorTable(0, particleUavIndex_);

    // Dispatch
    commandList->Dispatch(1, 1, 1); // Only 1 thread group needed (1024 threads inside)
    
    // Wait for UAV writes to complete
    dxCommon_->UAVBarrier(particleBuffer_.Get());
    dxCommon_->UAVBarrier(freeListIndexBuffer_.Get());
    dxCommon_->UAVBarrier(freeListBuffer_.Get());
}

void GPUParticleManager::SetEmitParams(const MyMath::Vector3& position, uint32_t count, float radius, float frequency) {
    emitterSphereData_->translate = position;
    emitterSphereData_->count = count;
    emitterSphereData_->radius = radius;
    emitterSphereData_->frequency = frequency;
}

void GPUParticleManager::Emit() {
    emitterSphereData_->emit = 1;
}

void GPUParticleManager::Update() {
    time_ += perFrameData_->deltaTime;
    perFrameData_->time = time_;

    // Emit Logic (CPU side)
    emitterSphereData_->frequencyTime += perFrameData_->deltaTime;
    if (emitterSphereData_->frequency <= emitterSphereData_->frequencyTime) {
        emitterSphereData_->frequencyTime -= emitterSphereData_->frequency;
        emitterSphereData_->emit = 1;
    } else {
        emitterSphereData_->emit = 0;
    }

    auto commandList = dxCommon_->GetCommandList();
    
    //---------------------------------------------------------
    // EMIT PASS
    //---------------------------------------------------------
    commandList->SetPipelineState(emitPipelineState_.Get());
    commandList->SetComputeRootSignature(computeRootSignature_.Get());
    
    // RootParam 0: UAV Table
    // The SRVs were allocated sequentially: particle, freeListIndex, freeList
    // So if particleUavIndex_ was allocated first, we bind that as the table start.
    // NOTE: In SrvManager they are allocated using Allocate() which returns sequentially incremented index.
    // But to be completely safe, we should ensure they are contiguous. Let's assume they are since we allocated them back to back.
    // Wait, in SrvManager, GetGPUDescriptorHandle increments by descriptor size. So yes, they are contiguous.
    srvManager_->SetGraphicsRootDescriptorTable(0, particleUavIndex_);
    
    // RootParam 1: Emitter CBV
    commandList->SetComputeRootConstantBufferView(1, emitterSphereBuffer_->GetGPUVirtualAddress());
    
    // RootParam 2: PerFrame CBV
    commandList->SetComputeRootConstantBufferView(2, perFrameBuffer_->GetGPUVirtualAddress());
    
    // Dispatch
    commandList->Dispatch(1, 1, 1);
    
    // UAV Barrier
    dxCommon_->UAVBarrier(particleBuffer_.Get());
    dxCommon_->UAVBarrier(freeListIndexBuffer_.Get());
    dxCommon_->UAVBarrier(freeListBuffer_.Get());

    //---------------------------------------------------------
    // UPDATE PASS
    //---------------------------------------------------------
    commandList->SetPipelineState(updatePipelineState_.Get());
    
    // Dispatch (1024 threads)
    commandList->Dispatch(1, 1, 1);
    
    // UAV Barrier
    dxCommon_->UAVBarrier(particleBuffer_.Get());
    dxCommon_->UAVBarrier(freeListIndexBuffer_.Get());
    dxCommon_->UAVBarrier(freeListBuffer_.Get());
}

void GPUParticleManager::Draw(const MyMath::Matrix4x4& viewProjection, const MyMath::Matrix4x4& billboardMatrix) {
    auto commandList = dxCommon_->GetCommandList();

    // Update PerView Buffer
    perViewData_->viewProjection = viewProjection;
    perViewData_->billboardMatrix = billboardMatrix;

    commandList->SetPipelineState(graphicsPipelineState_.Get());
    commandList->SetGraphicsRootSignature(graphicsRootSignature_.Get());
    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
    commandList->IASetVertexBuffers(0, 1, &vertexBufferView_);

    // RootParam 0: Particle SRV Table
    srvManager_->SetGraphicsRootDescriptorTable(0, particleSrvIndex_);
    
    // RootParam 1: PerView CBV
    commandList->SetGraphicsRootConstantBufferView(1, perViewBuffer_->GetGPUVirtualAddress());
    
    // RootParam 2: Texture SRV Table
    srvManager_->SetGraphicsRootDescriptorTable(2, textureSrvIndex_);

    // Draw Instanced
    commandList->DrawInstanced(4, kMaxParticles, 0, 0);
}
