#include "ParticleManager.h"
#include "TextureManager.h"
#include "CameraManager.h"
#include <cassert>
#include <d3dcompiler.h>

#pragma comment(lib, "d3dcompiler.lib")

using namespace MyMath;

std::unique_ptr<ParticleManager> ParticleManager::instance_ = nullptr;

ParticleManager* ParticleManager::GetInstance() {
	if (instance_ == nullptr) {
		instance_.reset(new ParticleManager());
	}
	return instance_.get();
}

void ParticleManager::Initialize(DirectXCommon* dxCommon, SrvManager* srvManager) {
	assert(dxCommon);
	assert(srvManager);

	dxCommon_ = dxCommon;
	srvManager_ = srvManager;

	std::random_device seed_gen;
	randomEngine_.seed(seed_gen());

	CreateRootSignature();
	CreateGraphicsPipeline();

	CreateModel();
}

void ParticleManager::Update() {
	Camera* camera = CameraManager::GetInstance()->GetActiveCamera();
	Matrix4x4 viewMatrix = camera->GetViewMatrix();
	Matrix4x4 projectionMatrix = camera->GetProjectionMatrix();
	Matrix4x4 viewProjectionMatrix = camera->GetViewProjectionMatrix();


	Matrix4x4 cameraWorldMatrix = camera->GetWorldMatrix();
	Matrix4x4 billboardMatrix = Identity4x4();
	billboardMatrix.m[0][0] = cameraWorldMatrix.m[0][0];
	billboardMatrix.m[0][1] = cameraWorldMatrix.m[0][1];
	billboardMatrix.m[0][2] = cameraWorldMatrix.m[0][2];
	billboardMatrix.m[1][0] = cameraWorldMatrix.m[1][0];
	billboardMatrix.m[1][1] = cameraWorldMatrix.m[1][1];
	billboardMatrix.m[1][2] = cameraWorldMatrix.m[1][2];
	billboardMatrix.m[2][0] = cameraWorldMatrix.m[2][0];
	billboardMatrix.m[2][1] = cameraWorldMatrix.m[2][1];
	billboardMatrix.m[2][2] = cameraWorldMatrix.m[2][2];


	for (auto& [name, group] : particleGroups_) {

		group.instanceCount = 0; 

		for (auto it = group.particles.begin(); it != group.particles.end();) {

			if (it->currentTime >= it->lifeTime) {
				it = group.particles.erase(it);
				continue;
			}

			Vector3 fieldAcceleration = { 0.0f, 0.0f, 0.0f };
			for (const auto& [fieldName, field] : accelerationFields_) {
				if (it->transform.translate.x >= field.area.min.x && it->transform.translate.x <= field.area.max.x &&
					it->transform.translate.y >= field.area.min.y && it->transform.translate.y <= field.area.max.y &&
					it->transform.translate.z >= field.area.min.z && it->transform.translate.z <= field.area.max.z) {
					fieldAcceleration = Add(fieldAcceleration, field.acceleration);
				}
			}

			it->velocity = Add(it->velocity, Add(it->acceleration, fieldAcceleration));
			it->transform.translate = Add(it->transform.translate, it->velocity);

			it->currentTime += 1.0f / 60.0f; 

			if (group.instanceCount < kMaxInstanceCount_) {

				Matrix4x4 scaleMatrix = MakeScaleMatrix(it->transform.scale);
				Matrix4x4 translateMatrix = MakeTranslateMatrix(it->transform.translate);
				Matrix4x4 worldMatrix = Multiply(scaleMatrix, Multiply(billboardMatrix, translateMatrix));

				Matrix4x4 wvp = Multiply(worldMatrix, viewProjectionMatrix);

				group.instancingDataPtr[group.instanceCount].WVP = wvp;
				group.instancingDataPtr[group.instanceCount].color = it->color;

				group.instanceCount++;
			}

			++it;
		}
	}
}

void ParticleManager::Draw() {
	ID3D12GraphicsCommandList* commandList = dxCommon_->GetCommandList();
	srvManager_->PreDraw();
	commandList->SetGraphicsRootSignature(rootSignature_.Get());

	// commandList->SetPipelineState(graphicsPipelineState_.Get());

	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	commandList->IASetVertexBuffers(0, 1, &vertexBufferView_);

	BlendMode currentBlendMode = BlendMode::kCountOf; 

	for (auto& [name, group] : particleGroups_) {
		if (group.instanceCount == 0) continue;

		if (group.blendMode != currentBlendMode) {
			commandList->SetPipelineState(graphicsPipelineStates_[static_cast<size_t>(group.blendMode)].Get());
			currentBlendMode = group.blendMode;
		}

		D3D12_GPU_DESCRIPTOR_HANDLE textureHandle = srvManager_->GetGPUDescriptorHandle(group.textureSrvIndex);
		commandList->SetGraphicsRootDescriptorTable(0, textureHandle);

		D3D12_GPU_DESCRIPTOR_HANDLE instancingHandle = srvManager_->GetGPUDescriptorHandle(group.instancingSrvIndex);
		commandList->SetGraphicsRootDescriptorTable(1, instancingHandle);

		commandList->DrawInstanced(6, group.instanceCount, 0, 0);
	}
}

void ParticleManager::SetBlendMode(const std::string& name, BlendMode mode) {
	if (particleGroups_.contains(name)) {
		particleGroups_[name].blendMode = mode;
	}
}

void ParticleManager::AddAccelerationField(const std::string& name, const Vector3& acceleration, const AABB& area) {
	AccelerationField field;
	field.acceleration = acceleration;
	field.area = area;
	accelerationFields_[name] = field;
}


void ParticleManager::Finalize() {
	particleGroups_.clear();
	instance_.reset();
}

void ParticleManager::CreateParticleGroup(const std::string& name, const std::string& textureFilePath) {
	assert(particleGroups_.contains(name) == false);

	ParticleGroup& group = particleGroups_[name];

	group.textureFilePath = textureFilePath;

	TextureManager::GetInstance()->LoadTexture(textureFilePath);
	group.textureSrvIndex = TextureManager::GetInstance()->GetTextureIndexByFilePath(textureFilePath);

	uint32_t sizeInBytes = sizeof(ParticleInstancingData) * kMaxInstanceCount_;
	group.instancingResource = dxCommon_->CreateBufferResource(sizeInBytes);

	group.instancingResource->Map(0, nullptr, reinterpret_cast<void**>(&group.instancingDataPtr));

	group.instancingSrvIndex = srvManager_->Allocate();

	srvManager_->CreateSRVforStructuredBuffer(
		group.instancingSrvIndex,
		group.instancingResource.Get(),
		kMaxInstanceCount_,
		sizeof(ParticleInstancingData)
	);

	group.instanceCount = 0;
}

void ParticleManager::Emit(const std::string& name, const Vector3& position, uint32_t count) {
	ParticleParameters params; 
	Emit(name, position, params, count);
}

void ParticleManager::Emit(const std::string& name, const Vector3& position, const ParticleParameters& params, uint32_t count) {
	assert(particleGroups_.contains(name));

	ParticleGroup& group = particleGroups_[name];

	std::uniform_real_distribution<float> distVelX(params.minVelocity.x, params.maxVelocity.x);
	std::uniform_real_distribution<float> distVelY(params.minVelocity.y, params.maxVelocity.y);
	std::uniform_real_distribution<float> distVelZ(params.minVelocity.z, params.maxVelocity.z);

	std::uniform_real_distribution<float> distColorR(params.minColor.x, params.maxColor.x);
	std::uniform_real_distribution<float> distColorG(params.minColor.y, params.maxColor.y);
	std::uniform_real_distribution<float> distColorB(params.minColor.z, params.maxColor.z);
	std::uniform_real_distribution<float> distColorA(params.minColor.w, params.maxColor.w);

	std::uniform_real_distribution<float> distTime(params.minLifeTime, params.maxLifeTime);

	std::uniform_real_distribution<float> distPos(-1.0f, 1.0f); 

	for (uint32_t i = 0; i < count; ++i) {
		Particle particle{};
		particle.transform.scale = { 1.0f, 1.0f, 1.0f };
		particle.transform.rotate = { 0.0f, 0.0f, 0.0f };

		Vector3 randomPos = { distPos(randomEngine_), distPos(randomEngine_), distPos(randomEngine_) };
		particle.transform.translate = Add(position, randomPos);

		particle.velocity = { distVelX(randomEngine_), distVelY(randomEngine_), distVelZ(randomEngine_) };

		particle.acceleration = params.acceleration;

		particle.color = { distColorR(randomEngine_), distColorG(randomEngine_), distColorB(randomEngine_), distColorA(randomEngine_) };
		particle.lifeTime = distTime(randomEngine_);
		particle.currentTime = 0.0f;

		group.particles.push_back(particle);
	}
}

void ParticleManager::CreateModel() {
	VertexData vertices[6];


	Vector4 leftBottom = { -0.5f, -0.5f, 0.0f, 1.0f };
	Vector4 leftTop = { -0.5f,  0.5f, 0.0f, 1.0f };
	Vector4 rightBottom = { 0.5f, -0.5f, 0.0f, 1.0f };
	Vector4 rightTop = { 0.5f,  0.5f, 0.0f, 1.0f };

	// UV
	Vector2 uvLeftBottom = { 0.0f, 1.0f };
	Vector2 uvLeftTop = { 0.0f, 0.0f };
	Vector2 uvRightBottom = { 1.0f, 1.0f };
	Vector2 uvRightTop = { 1.0f, 0.0f };

	vertices[0].position = leftBottom;  vertices[0].texcoord = uvLeftBottom;
	vertices[1].position = leftTop;     vertices[1].texcoord = uvLeftTop;
	vertices[2].position = rightBottom; vertices[2].texcoord = uvRightBottom;
	vertices[3].position = leftTop;     vertices[3].texcoord = uvLeftTop;
	vertices[4].position = rightTop;    vertices[4].texcoord = uvRightTop;
	vertices[5].position = rightBottom; vertices[5].texcoord = uvRightBottom;

	uint32_t sizeInBytes = sizeof(VertexData) * 6;

	vertexBuffer_ = dxCommon_->CreateBufferResource(sizeInBytes);

	vertexBufferView_.BufferLocation = vertexBuffer_->GetGPUVirtualAddress();
	vertexBufferView_.SizeInBytes = sizeInBytes;
	vertexBufferView_.StrideInBytes = sizeof(VertexData);

	VertexData* data = nullptr;
	vertexBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&data));
	std::memcpy(data, vertices, sizeInBytes);
	vertexBuffer_->Unmap(0, nullptr);
}


void ParticleManager::CreateRootSignature() {
	D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
	descriptionRootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	D3D12_DESCRIPTOR_RANGE descriptorRange[1] = {};
	descriptorRange[0].BaseShaderRegister = 0; 
	descriptorRange[0].NumDescriptors = 1;
	descriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	D3D12_DESCRIPTOR_RANGE descriptorRangeInstancing[1] = {};
	descriptorRangeInstancing[0].BaseShaderRegister = 1;
	descriptorRangeInstancing[0].NumDescriptors = 1;
	descriptorRangeInstancing[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descriptorRangeInstancing[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	D3D12_ROOT_PARAMETER rootParameters[2] = {};

	rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameters[0].DescriptorTable.pDescriptorRanges = descriptorRange;
	rootParameters[0].DescriptorTable.NumDescriptorRanges = _countof(descriptorRange);

	rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
	rootParameters[1].DescriptorTable.pDescriptorRanges = descriptorRangeInstancing;
	rootParameters[1].DescriptorTable.NumDescriptorRanges = _countof(descriptorRangeInstancing);

	D3D12_STATIC_SAMPLER_DESC staticSamplers[1] = {};
	staticSamplers[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	staticSamplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	staticSamplers[0].MaxLOD = D3D12_FLOAT32_MAX;
	staticSamplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	descriptionRootSignature.pParameters = rootParameters;
	descriptionRootSignature.NumParameters = _countof(rootParameters);
	descriptionRootSignature.pStaticSamplers = staticSamplers;
	descriptionRootSignature.NumStaticSamplers = _countof(staticSamplers);

	Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob = nullptr;
	Microsoft::WRL::ComPtr<ID3DBlob> errorBlob = nullptr;
	HRESULT hr = D3D12SerializeRootSignature(&descriptionRootSignature, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
	if (FAILED(hr)) {
		assert(false);
	}
	hr = dxCommon_->GetDevice()->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature_));
	assert(SUCCEEDED(hr));
}



void ParticleManager::CreateGraphicsPipeline() {
	Microsoft::WRL::ComPtr<IDxcBlob> vertexShaderBlob = dxCommon_->CompileShader(L"./assets/shaders/Particle.VS.hlsl", L"vs_6_0");
	Microsoft::WRL::ComPtr<IDxcBlob> pixelShaderBlob = dxCommon_->CompileShader(L"./assets/shaders/Particle.PS.hlsl", L"ps_6_0");

	// InputLayout
	D3D12_INPUT_ELEMENT_DESC inputElementDescs[2] = {};
	inputElementDescs[0].SemanticName = "POSITION";
	inputElementDescs[0].SemanticIndex = 0;
	inputElementDescs[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	inputElementDescs[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	inputElementDescs[1].SemanticName = "TEXCOORD";
	inputElementDescs[1].SemanticIndex = 0;
	inputElementDescs[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	inputElementDescs[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

	D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc{};
	graphicsPipelineStateDesc.pRootSignature = rootSignature_.Get();
	graphicsPipelineStateDesc.InputLayout = { inputElementDescs, _countof(inputElementDescs) };
	graphicsPipelineStateDesc.VS = { vertexShaderBlob->GetBufferPointer(), vertexShaderBlob->GetBufferSize() };
	graphicsPipelineStateDesc.PS = { pixelShaderBlob->GetBufferPointer(), pixelShaderBlob->GetBufferSize() };

	graphicsPipelineStateDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE; 
	graphicsPipelineStateDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;

	graphicsPipelineStateDesc.DepthStencilState.DepthEnable = false;
	graphicsPipelineStateDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	graphicsPipelineStateDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;

	graphicsPipelineStateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	graphicsPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	graphicsPipelineStateDesc.NumRenderTargets = 1;
	graphicsPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	graphicsPipelineStateDesc.SampleDesc.Count = 1;
	graphicsPipelineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;

	for (size_t i = 0; i < static_cast<size_t>(BlendMode::kCountOf); ++i) {
		BlendMode mode = static_cast<BlendMode>(i);
		graphicsPipelineStateDesc.BlendState = GetBlendDesc(mode);

		HRESULT hr = dxCommon_->GetDevice()->CreateGraphicsPipelineState(&graphicsPipelineStateDesc, IID_PPV_ARGS(&graphicsPipelineStates_[i]));
		assert(SUCCEEDED(hr));
	}
}
