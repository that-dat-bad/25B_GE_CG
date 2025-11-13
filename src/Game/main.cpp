#include <Windows.h>
#include <cstdint>
#include <string>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <cassert>
#include <format>
#include <strsafe.h>
#include <dxcapi.h>
#include <vector>
#include "../engine/base/Math/Matrix4x4.h"
#include "../engine/base/Math/Vector3.h"
#include "../engine/base/Math/Vector2.h"
#include <math.h>
#define _USE_MATH_DEFINES
#include <fstream>
#include <sstream>
#include <map>
#include "../engine/Graphics/DebugCamera.h"
#include "../engine/base/winApp.h"
#include "../engine/io/Input.h"
#include"../engine/base/logger.h"
using namespace logger;
#include"../engine/base/StringUtility.h"
using namespace StringUtility;

#include"../engine/Graphics/DirectXCommon.h"
#include"../engine/Graphics/SpriteCommon.h"
#include"../engine/Graphics/Sprite.h"

// debug用のヘッダ
#include <DbgHelp.h>
#pragma comment(lib, "Dbghelp.lib")

#include <dxgidebug.h>
#pragma comment(lib, "dxguid.lib")

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

#pragma comment(lib,"dxcompiler.lib")


#include "../../external/imgui/imgui.h"
#include "../../external/imgui/imgui_impl_dx12.h"
#include "../../external/imgui/imgui_impl_win32.h"

#include "../../external/DirectXTex/DirectXTex.h"
#include "../../external/DirectXTex/d3dx12.h"

#include <xaudio2.h>
#pragma comment(lib, "xaudio2.lib")

#include <Xinput.h>
#pragma comment(lib, "xinput.lib")

// 構造体の定義
struct Vector4 { float x, y, z, w; };

struct Transform {
	Vector3 scale;
	Vector3 rotate;
	Vector3 translate;
};

struct VertexData {
	Vector4 position;
	Vector2 texcoord;
	Vector3 normal;
};


struct Material {
	Vector4 color;
	int32_t enableLighting;
	float shininess;
	float padding[2];
	Matrix4x4 uvTransform; // UV変換行列
};

struct TransformationMatrix {
	Matrix4x4 WVP;
	Matrix4x4 World;
};

struct DirectionalLight {
	Vector4 color;
	Vector3 direction;
	float intensity;
};


struct LightingSettings {
	int32_t lightingModel; // 0: Lambert, 1: Half-Lambert
	float padding[3];
};

struct MaterialData {
	std::string textureFilePath;
	std::string name; // マテリアル名を追加
};

// 各メッシュの情報を保持する構造体
struct MeshObject {
	std::string name;
	std::vector<VertexData> vertices;
	MaterialData material; // このメッシュに適用されるマテリアル
	Transform transform; // このメッシュ固有のSRT
	Transform uvTransform; // メッシュ固有のUV変換
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource;
	Material* materialData = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> wvpResource;
	TransformationMatrix* wvpData = nullptr;
	int textureAssetIndex = 0; // このメッシュが使用するテクスチャのインデックス
	bool hasUV = false; // このメッシュがUVを持つか
};

// 読み込んだモデルアセット (複数のメッシュを含む)
struct ModelData {
	std::string name;
	std::vector<MeshObject> meshes; // 複数のメッシュを保持
};

// 読み込んだテクスチャアセット
struct TextureAsset {
	std::string name;
	Microsoft::WRL::ComPtr<ID3D12Resource> resource;
	D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle;
	// テクスチャのメタデータを保持するためのフィールドを追加
	DirectX::TexMetadata metadata;
};

// 読み込んだモデルアセット (ModelDataを保持)
struct ModelAsset {
	ModelData modelData;
	// ModelAssetレベルでの頂点バッファは不要になる
};


// シーン内のオブジェクト (ModelAssetを参照し、その中のメッシュを管理)
struct GameObject {
	Transform transform; // オブジェクト全体の変換 (各メッシュに適用される)
	int modelAssetIndex = 0; // どのModelAssetを使用するか
};

struct ChunkHeader {
	char id[4];
	int32_t size;
};

struct RiffHeader {
	ChunkHeader chunk;
	char type[4];
};

struct FormatChunk {
	ChunkHeader chunk;
	WAVEFORMATEX fmt;
};

struct SoundData {
	WAVEFORMATEX wfex;
	BYTE* pBuffer;
	unsigned int buffersize;
};


#pragma region 関数群



static LONG WINAPI ExportDump(EXCEPTION_POINTERS* exception) {
	SYSTEMTIME time;
	GetLocalTime(&time);
	wchar_t filePath[MAX_PATH] = { 0 };
	CreateDirectory(L"./Dump", nullptr);
	StringCchPrintfW(filePath, MAX_PATH, L"./Dump/%04d-%02d-%02d-%02d-%02d.dmp",
		time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute);
	HANDLE dumpFileHandle = CreateFile(filePath, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_WRITE | FILE_SHARE_READ, 0, CREATE_ALWAYS, 0, 0);

	DWORD processId = GetCurrentProcessId();
	DWORD threadId = GetCurrentThreadId();

	MINIDUMP_EXCEPTION_INFORMATION minidumpInformation{ 0 };
	minidumpInformation.ThreadId = threadId;
	minidumpInformation.ExceptionPointers = exception;
	minidumpInformation.ClientPointers = TRUE;

	MiniDumpWriteDump(GetCurrentProcess(), processId, dumpFileHandle, MiniDumpNormal, &minidumpInformation, nullptr, nullptr);
	return EXCEPTION_EXECUTE_HANDLER;
}


// 複数のマテリアルを読み込むためのマップ
std::map<std::string, MaterialData> LoadMaterialTemplates(const std::string& directoryPath, const std::string& filename) {
	std::map<std::string, MaterialData> materials;
	std::string currentMaterialName;
	std::string line;
	std::ifstream file(directoryPath + "/" + filename);
	assert(file.is_open());

	while (std::getline(file, line)) {
		std::istringstream s(line);
		std::string identifier;
		s >> identifier;

		if (identifier == "newmtl") {
			s >> currentMaterialName;
			materials[currentMaterialName].name = currentMaterialName;
		} else if (identifier == "map_Kd" && !currentMaterialName.empty()) {
			std::string textureFileName;
			s >> textureFileName;
			materials[currentMaterialName].textureFilePath = directoryPath + "/" + textureFileName;
		}
	}
	return materials;
}

// OBJファイルを読み込み、複数のメッシュとして解析する関数
ModelData LoadObjFile(const std::string& directoryPath, const std::string& filename, DirectXCommon* dxCommon) {
	ModelData modeldata;
	modeldata.name = filename;
	std::vector<Vector4> positions;
	std::vector<Vector3> normals;
	std::vector<Vector2> texcoords;
	std::string line;
	std::ifstream file(directoryPath + "/" + filename);
	assert(file.is_open());

	std::map<std::string, MaterialData> loadedMaterials;
	MeshObject currentMesh;
	bool firstMesh = true;

	while (std::getline(file, line)) {
		std::string identifier;
		std::istringstream s(line);
		s >> identifier;

		if (identifier == "o" || identifier == "g") { // 新しいオブジェクトまたはグループの開始
			if (!firstMesh) {
				// 以前のメッシュを保存
				if (!currentMesh.vertices.empty()) {
					// メッシュの頂点バッファを作成
					currentMesh.vertexBuffer = dxCommon->CreateBufferResource(sizeof(VertexData) * currentMesh.vertices.size());
					VertexData* mappedData = nullptr;
					currentMesh.vertexBuffer->Map(0, nullptr, reinterpret_cast<void**>(&mappedData));
					std::memcpy(mappedData, currentMesh.vertices.data(), sizeof(VertexData) * currentMesh.vertices.size());
					currentMesh.vertexBuffer->Unmap(0, nullptr);

					currentMesh.vertexBufferView.BufferLocation = currentMesh.vertexBuffer->GetGPUVirtualAddress();
					currentMesh.vertexBufferView.SizeInBytes = UINT(sizeof(VertexData) * currentMesh.vertices.size());
					currentMesh.vertexBufferView.StrideInBytes = sizeof(VertexData);

					// マテリアルとWVPのリソースを作成
					currentMesh.materialResource = dxCommon->CreateBufferResource(sizeof(Material));
					currentMesh.materialResource->Map(0, nullptr, reinterpret_cast<void**>(&currentMesh.materialData));
					currentMesh.materialData->color = { 1.0f, 1.0f, 1.0f, 1.0f }; // デフォルト色
					currentMesh.materialData->enableLighting = 1;
					currentMesh.materialData->shininess = 0.0f;
					currentMesh.materialData->uvTransform = Identity4x4(); // UV変換を単位行列に初期化

					currentMesh.wvpResource = dxCommon->CreateBufferResource(sizeof(TransformationMatrix));
					currentMesh.wvpResource->Map(0, nullptr, reinterpret_cast<void**>(&currentMesh.wvpData));
					currentMesh.transform = { {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f} }; // デフォルト変換
					currentMesh.uvTransform = { {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f} }; // UV変換の初期化

					modeldata.meshes.push_back(currentMesh);
				}
			}
			firstMesh = false;
			currentMesh = MeshObject(); // 新しいメッシュを初期化
			s >> currentMesh.name; // メッシュ名を設定
			currentMesh.transform = { {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f} }; // デフォルト変換
			currentMesh.uvTransform = { {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f} }; // UV変換の初期化
			currentMesh.hasUV = false; // 初期化
		} else if (identifier == "v") {
			Vector4 position;
			s >> position.x >> position.y >> position.z;
			position.w = 1.0f;
			// 座標系の変換
			position.x *= -1.0f;
			positions.push_back(position);
		} else if (identifier == "vt") {
			Vector2 texcoord;
			s >> texcoord.x >> texcoord.y;
			// V方向の反転
			texcoord.y = 1.0f - texcoord.y;
			texcoords.push_back(texcoord);
			currentMesh.hasUV = true; // このメッシュはUVを持つ
		} else if (identifier == "vn") {
			Vector3 normal;
			s >> normal.x >> normal.y >> normal.z;
			// 座標系の変換
			normal.x *= -1.0f;
			normals.push_back(normal);
		} else if (identifier == "f") {
			VertexData faceVertices[3];
			for (int32_t faceVertex = 0; faceVertex < 3; ++faceVertex) {
				std::string vertexDefinition;
				s >> vertexDefinition;

				uint32_t elementIndices[3] = { 0, 0, 0 };

				size_t first_slash = vertexDefinition.find('/');
				elementIndices[0] = std::stoi(vertexDefinition.substr(0, first_slash));

				if (first_slash != std::string::npos) {
					if (vertexDefinition[first_slash + 1] != '/') {
						size_t second_slash = vertexDefinition.find('/', first_slash + 1);
						elementIndices[1] = std::stoi(vertexDefinition.substr(first_slash + 1, second_slash - (first_slash + 1)));
					}
					size_t second_slash = vertexDefinition.find('/', first_slash + 1);
					if (second_slash != std::string::npos) {
						elementIndices[2] = std::stoi(vertexDefinition.substr(second_slash + 1));
					}
				}

				Vector4 position = positions[elementIndices[0] - 1];
				Vector2 texcoord = { 0.0f, 0.0f };
				if (elementIndices[1] != 0 && currentMesh.hasUV) { // currentMesh.hasUV を使用
					texcoord = texcoords[elementIndices[1] - 1];
				}
				Vector3 normal = (elementIndices[2] > 0 && !normals.empty()) ? normals[elementIndices[2] - 1] : Vector3{ 0.0f, 0.0f, 1.0f };

				faceVertices[faceVertex] = { position, texcoord, normal };
			}
			currentMesh.vertices.push_back(faceVertices[0]);
			currentMesh.vertices.push_back(faceVertices[2]);
			currentMesh.vertices.push_back(faceVertices[1]);
		} else if (identifier == "mtllib") {
			std::string materialFileName;
			s >> materialFileName;
			loadedMaterials = LoadMaterialTemplates(directoryPath, materialFileName);
		} else if (identifier == "usemtl") {
			std::string materialName;
			s >> materialName;
			if (loadedMaterials.count(materialName)) {
				currentMesh.material = loadedMaterials[materialName];
			}
		}
	}
	// ファイルの終わりに残った最後のメッシュを保存
	if (!currentMesh.vertices.empty()) {
		currentMesh.vertexBuffer = dxCommon->CreateBufferResource(sizeof(VertexData) * currentMesh.vertices.size());
		VertexData* mappedData = nullptr;
		currentMesh.vertexBuffer->Map(0, nullptr, reinterpret_cast<void**>(&mappedData));
		std::memcpy(mappedData, currentMesh.vertices.data(), sizeof(VertexData) * currentMesh.vertices.size());
		currentMesh.vertexBuffer->Unmap(0, nullptr);

		currentMesh.vertexBufferView.BufferLocation = currentMesh.vertexBuffer->GetGPUVirtualAddress();
		currentMesh.vertexBufferView.SizeInBytes = UINT(sizeof(VertexData) * currentMesh.vertices.size());
		currentMesh.vertexBufferView.StrideInBytes = sizeof(VertexData);

		currentMesh.materialResource = dxCommon->CreateBufferResource(sizeof(Material));
		currentMesh.materialResource->Map(0, nullptr, reinterpret_cast<void**>(&currentMesh.materialData));
		currentMesh.materialData->color = { 1.0f, 1.0f, 1.0f, 1.0f }; // デフォルト色
		currentMesh.materialData->enableLighting = 1;
		currentMesh.materialData->shininess = 0.0f;
		currentMesh.materialData->uvTransform = Identity4x4(); // UV変換を単位行列に初期化

		currentMesh.wvpResource = dxCommon->CreateBufferResource(sizeof(TransformationMatrix));
		currentMesh.wvpResource->Map(0, nullptr, reinterpret_cast<void**>(&currentMesh.wvpData));
		currentMesh.transform = { {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f} }; // デフォルト変換
		currentMesh.uvTransform = { {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f} }; // UV変換の初期化

		modeldata.meshes.push_back(currentMesh);
	}
	return modeldata;
}


SoundData SoundLoadWave(const char* filename) {
	std::ifstream file;
	file.open(filename, std::ios_base::binary);
	assert(file.is_open());
	RiffHeader riff;
	file.read((char*)&riff, sizeof(riff));
	if (strncmp(riff.chunk.id, "RIFF", 4) != 0) assert(0);
	if (strncmp(riff.type, "WAVE", 4) != 0) assert(0);
	FormatChunk format = {};
	file.read((char*)&format, sizeof(ChunkHeader));
	if (strncmp(format.chunk.id, "fmt ", 4) != 0) assert(0);
	file.read((char*)&format.fmt, format.chunk.size);
	ChunkHeader data;
	file.read((char*)&data, sizeof(data));
	if (strncmp(data.id, "JUNK", 4) == 0) {
		file.seekg(data.size, std::ios_base::cur);
		file.read((char*)&data, sizeof(data));
	}
	if (strncmp(data.id, "data", 4) != 0) assert(0);
	char* pBuffer = new char[data.size];
	file.read(pBuffer, data.size);
	file.close();
	SoundData soundData = {};
	soundData.wfex = format.fmt;
	soundData.pBuffer = reinterpret_cast<BYTE*>(pBuffer);
	soundData.buffersize = data.size;
	return soundData;
}

void SoundUnload(SoundData* soundData) {
	delete[] soundData->pBuffer;
	soundData->pBuffer = nullptr;
	soundData->buffersize = 0;
	soundData->wfex = {};
}

void SoundPlayWave(IXAudio2* xAudio2, const SoundData& soundData) {
	IXAudio2SourceVoice* pSourceVoice = nullptr;
	HRESULT result = xAudio2->CreateSourceVoice(&pSourceVoice, &soundData.wfex);
	assert(SUCCEEDED(result));
	XAUDIO2_BUFFER buf = {};
	buf.pAudioData = soundData.pBuffer;
	buf.AudioBytes = soundData.buffersize;
	buf.Flags = XAUDIO2_END_OF_STREAM;
	result = pSourceVoice->SubmitSourceBuffer(&buf);
	result = pSourceVoice->Start(0);
}
#pragma endregion

// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int) {
#pragma region 基盤の初期化処理
	WinApp* winApp = new WinApp();
	winApp->Initialize();
	SetUnhandledExceptionFilter(ExportDump);

	Input* input = nullptr;
	input = new Input();
	input->Initialize(hInstance, winApp->GetHwnd());

	// DirectXCommonの初期化
	DirectXCommon* dxCommon = new DirectXCommon();
	dxCommon->Initialize(winApp);

	//Sprite共通部の初期化
	SpriteCommon* spriteCommon = new SpriteCommon();
	spriteCommon->Initialize(dxCommon);

	// DirectXCommonから必要なオブジェクトを取得
	ID3D12Device* device = dxCommon->GetDevice();
	ID3D12GraphicsCommandList* commandList = dxCommon->GetCommandList();
	ID3D12CommandQueue* commandQueue = dxCommon->GetCommandQueue();
#pragma endregion


	// RootSignature
	D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
	descriptionRootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	D3D12_DESCRIPTOR_RANGE descriptorRange[1] = {};
	descriptorRange[0].BaseShaderRegister = 0;
	descriptorRange[0].NumDescriptors = 1;
	descriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	D3D12_ROOT_PARAMETER rootParameters[5] = {};
	rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameters[0].Descriptor.ShaderRegister = 0; // for Material
	rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
	rootParameters[1].Descriptor.ShaderRegister = 1; // for WVP
	rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameters[2].DescriptorTable.pDescriptorRanges = descriptorRange;
	rootParameters[2].DescriptorTable.NumDescriptorRanges = _countof(descriptorRange);
	rootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameters[3].Descriptor.ShaderRegister = 1; // for DirectionalLight
	rootParameters[4].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[4].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameters[4].Descriptor.ShaderRegister = 2; // for LightingSettings

	D3D12_STATIC_SAMPLER_DESC staticSamplers[1] = {};
	staticSamplers[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	staticSamplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	staticSamplers[0].MaxLOD = D3D12_FLOAT32_MAX;
	staticSamplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	descriptionRootSignature.pStaticSamplers = staticSamplers;
	descriptionRootSignature.NumStaticSamplers = _countof(staticSamplers);

	descriptionRootSignature.pParameters = rootParameters;
	descriptionRootSignature.NumParameters = _countof(rootParameters);

	Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob = nullptr;
	Microsoft::WRL::ComPtr<ID3DBlob> errorBlob = nullptr;
	HRESULT hr = D3D12SerializeRootSignature(&descriptionRootSignature, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
	if (FAILED(hr)) {
		Log(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
		assert(false);
	}
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature = nullptr;
	hr = device->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature));
	assert(SUCCEEDED(hr));

	// PSO
	D3D12_INPUT_ELEMENT_DESC inputElementDescs[3] = {};
	inputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	inputElementDescs[1] = { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	inputElementDescs[2] = { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

	D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc{};
	graphicsPipelineStateDesc.pRootSignature = rootSignature.Get();
	graphicsPipelineStateDesc.InputLayout = { inputElementDescs, _countof(inputElementDescs) };

	Microsoft::WRL::ComPtr<IDxcBlob> vertexShaderBlob = dxCommon->CompileShader(L"./assets/shaders/Object3D.VS.hlsl", L"vs_6_0");
	Microsoft::WRL::ComPtr<IDxcBlob> pixelShaderBlob = dxCommon->CompileShader(L"./assets/shaders/Object3D.PS.hlsl", L"ps_6_0");
	graphicsPipelineStateDesc.VS = { vertexShaderBlob->GetBufferPointer(), vertexShaderBlob->GetBufferSize() };
	graphicsPipelineStateDesc.PS = { pixelShaderBlob->GetBufferPointer(), pixelShaderBlob->GetBufferSize() };

	D3D12_BLEND_DESC blendDesc{};
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	blendDesc.RenderTarget[0].BlendEnable = TRUE;
	blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
	graphicsPipelineStateDesc.BlendState = blendDesc;

	D3D12_RASTERIZER_DESC rasterizerDesc{};
	rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
	graphicsPipelineStateDesc.RasterizerState = rasterizerDesc;

	D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
	depthStencilDesc.DepthEnable = true;
	depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	graphicsPipelineStateDesc.DepthStencilState = depthStencilDesc;
	graphicsPipelineStateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

	graphicsPipelineStateDesc.NumRenderTargets = 1;
	graphicsPipelineStateDesc.RTVFormats[0] = dxCommon->GetRTVFormat();
	graphicsPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	graphicsPipelineStateDesc.SampleDesc.Count = 1;
	graphicsPipelineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;

	Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineState = nullptr;
	hr = device->CreateGraphicsPipelineState(&graphicsPipelineStateDesc, IID_PPV_ARGS(&graphicsPipelineState));
	assert(SUCCEEDED(hr));

	// SRVディスクリプタヒープ
	ID3D12DescriptorHeap* srvDescriptorHeap = dxCommon->GetSRVDescriptorHeap();
	const uint32_t descriptorSizeSRV = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	// テクスチャ読み込み
	std::vector<TextureAsset> textureAssets;
	std::vector<std::string> texturePaths = {
		"assets/textures/uvchecker.png",
		"assets/textures/monsterBall.png",
		"assets/textures/checkerBoard.png",
	};

	// ImGui用に1つ予約
	uint32_t srvIndex = 1;



	// 初回コマンドリストの準備（テクスチャアップロード用）
	hr = dxCommon->GetCommandAllocator()->Reset();
	assert(SUCCEEDED(hr));
	hr = commandList->Reset(dxCommon->GetCommandAllocator(), nullptr);
	assert(SUCCEEDED(hr));


	for (const auto& path : texturePaths) {
		DirectX::ScratchImage mipImages = dxCommon->LoadTexture(path);
		const DirectX::TexMetadata& metadata = mipImages.GetMetadata();
		Microsoft::WRL::ComPtr<ID3D12Resource> textureResource = dxCommon->CreateTextureResource(device, metadata);
		dxCommon->UploadTextureData(textureResource.Get(), mipImages);
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
		if (metadata.format == DXGI_FORMAT_R8G8B8A8_UNORM) {
			srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
		} else {
			srvDesc.Format = metadata.format;
		}
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = UINT(metadata.mipLevels);

		TextureAsset newAsset;
		newAsset.name = path;
		newAsset.resource = textureResource;
		newAsset.cpuHandle = dxCommon->GetCPUDescriptorHandle(srvDescriptorHeap, descriptorSizeSRV, srvIndex);
		newAsset.gpuHandle = dxCommon->GetGPUDescriptorHandle(srvDescriptorHeap, descriptorSizeSRV, srvIndex);
		newAsset.metadata = metadata;
		device->CreateShaderResourceView(newAsset.resource.Get(), &srvDesc, newAsset.cpuHandle);
		textureAssets.push_back(newAsset);
		srvIndex++;
	}

	// モデル読み込み
	std::vector<ModelAsset> modelAssets;
	std::vector<std::string> modelPaths = {
		"sphere.obj",
		"plane.obj",
	};
	for (const auto& filename : modelPaths) {
		ModelData modelData = LoadObjFile("assets/models", filename, dxCommon);
		ModelAsset newAsset;
		newAsset.modelData = modelData;
		modelAssets.push_back(newAsset);
	}

	// ゲームオブジェクトの初期化
	std::vector<GameObject> gameObjects;
	GameObject obj1;
	obj1.transform = { {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f} };
	obj1.modelAssetIndex = 0; // Sphere
	gameObjects.push_back(obj1);

	GameObject obj2;
	obj2.transform = { {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {2.0f, 0.0f, 0.0f} };
	obj2.modelAssetIndex = 1; // Plane
	gameObjects.push_back(obj2);

	int selectedMeshIndex = 0;

	// スプライトの初期化
	Sprite* sprite = new Sprite();
	sprite->Initialize();

	Transform transformSprite{ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} };
	Transform uvTransformSprite{ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} };
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResourceSprite = dxCommon->CreateBufferResource(sizeof(VertexData) * 6);
	Microsoft::WRL::ComPtr<ID3D12Resource> indexResourceSprite = dxCommon->CreateBufferResource(sizeof(uint32_t) * 6);
	uint32_t* indexDataSprite = nullptr;
	indexResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&indexDataSprite));
	indexDataSprite[0] = 0; indexDataSprite[1] = 1; indexDataSprite[2] = 2;
	indexDataSprite[3] = 1; indexDataSprite[4] = 3; indexDataSprite[5] = 2;
	indexResourceSprite->Unmap(0, nullptr);
	VertexData* vertexDataSprite = nullptr;
	vertexResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&vertexDataSprite));
	float initialSpriteWidth = static_cast<float>(textureAssets[0].metadata.width);
	float initialSpriteHeight = static_cast<float>(textureAssets[0].metadata.height);

	vertexDataSprite[0].position = { 0.0f, initialSpriteHeight, 0.0f, 1.0f };   vertexDataSprite[0].texcoord = { 0.0f, 1.0f };
	vertexDataSprite[1].position = { 0.0f, 0.0f, 0.0f, 1.0f };      vertexDataSprite[1].texcoord = { 0.0f, 0.0f };
	vertexDataSprite[2].position = { initialSpriteWidth, initialSpriteHeight, 0.0f, 1.0f };  vertexDataSprite[2].texcoord = { 1.0f, 1.0f };
	vertexDataSprite[3].position = { initialSpriteWidth, 0.0f, 0.0f, 1.0f };    vertexDataSprite[3].texcoord = { 1.0f, 0.0f };
	vertexResourceSprite->Unmap(0, nullptr);
	D3D12_VERTEX_BUFFER_VIEW vertexBufferViewSprite{};
	vertexBufferViewSprite.BufferLocation = vertexResourceSprite->GetGPUVirtualAddress();
	vertexBufferViewSprite.SizeInBytes = sizeof(VertexData) * 4;
	vertexBufferViewSprite.StrideInBytes = sizeof(VertexData);
	D3D12_INDEX_BUFFER_VIEW indexBufferViewSprite{};
	indexBufferViewSprite.BufferLocation = indexResourceSprite->GetGPUVirtualAddress();
	indexBufferViewSprite.SizeInBytes = sizeof(uint32_t) * 6;
	indexBufferViewSprite.Format = DXGI_FORMAT_R32_UINT;
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResourceSprite = dxCommon->CreateBufferResource( sizeof(Material));
	Material* materialDataSprite = nullptr;
	materialResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&materialDataSprite));
	materialDataSprite->color = { 1.0f, 1.0f, 1.0f, 1.0f };
	materialDataSprite->enableLighting = 0;
	materialDataSprite->shininess = 0.0f;
	materialDataSprite->uvTransform = Identity4x4();

	Microsoft::WRL::ComPtr<ID3D12Resource> wvpResourceSprite = dxCommon->CreateBufferResource(sizeof(TransformationMatrix));
	TransformationMatrix* wvpDataSprite = nullptr;
	wvpResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&wvpDataSprite));
	int spriteTextureIndex = 0;
	bool isSpriteVisible = false;

	// ライトの初期化
	Microsoft::WRL::ComPtr<ID3D12Resource> directionalLightResource = dxCommon->CreateBufferResource(sizeof(DirectionalLight));
	DirectionalLight* directionalLightData = nullptr;
	directionalLightResource->Map(0, nullptr, reinterpret_cast<void**>(&directionalLightData));
	directionalLightData->color = { 1.0f, 1.0f, 1.0f, 1.0f };
	directionalLightData->direction = { 0.0f, 1.0f, 0.0f };
	directionalLightData->intensity = 1.0f;

	// ライティング設定の初期化
	Microsoft::WRL::ComPtr<ID3D12Resource> lightingSettingsResource = dxCommon->CreateBufferResource(sizeof(LightingSettings));
	LightingSettings* lightingSettingsData = nullptr;
	lightingSettingsResource->Map(0, nullptr, reinterpret_cast<void**>(&lightingSettingsData));
	lightingSettingsData->lightingModel = 0;

	// コマンドを確定させて待つ
	hr = commandList->Close();
	assert(SUCCEEDED(hr));
	ID3D12CommandList* commandLists[] = { commandList };
	commandQueue->ExecuteCommandLists(1, commandLists);

	dxCommon->IncrementFenceValue();
	commandQueue->Signal(dxCommon->GetFence(), dxCommon->GetFenceValue());
	if (dxCommon->GetFence()->GetCompletedValue() < dxCommon->GetFenceValue()) {
		dxCommon->GetFence()->SetEventOnCompletion(dxCommon->GetFenceValue(), dxCommon->GetFenceEvent());
		WaitForSingleObject(dxCommon->GetFenceEvent(), INFINITE);
	}
	hr = dxCommon->GetCommandAllocator()->Reset();
	assert(SUCCEEDED(hr));
	hr = commandList->Reset(dxCommon->GetCommandAllocator(), nullptr);
	assert(SUCCEEDED(hr));


	// 入力とカメラ
	DebugCamera g_debugCamera;

	// XAudio2の初期化
	IXAudio2* xAudio2 = nullptr;
	HRESULT hrXAudio2 = XAudio2Create(&xAudio2, 0, XAUDIO2_DEFAULT_PROCESSOR);
	assert(SUCCEEDED(hrXAudio2));

	IXAudio2MasteringVoice* masteringVoice = nullptr;
	hrXAudio2 = xAudio2->CreateMasteringVoice(&masteringVoice);
	assert(SUCCEEDED(hrXAudio2));

	// サウンドデータの読み込み
	SoundData alarmSound = SoundLoadWave("assets/sounds/Alarm01.wav");

	int selectedLightingOption = 0;

	while (true) {

		if (winApp->ProcessMessage()) {
			break;
		}

		ImGui_ImplDX12_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		input->Update();

		XINPUT_STATE gamepadState;
		ZeroMemory(&gamepadState, sizeof(XINPUT_STATE));
		DWORD dwResult = XInputGetState(0, &gamepadState);

		if (dwResult == ERROR_SUCCESS && !gameObjects.empty())
		{
			float rotationSpeed = 0.05f;
			if (abs(gamepadState.Gamepad.sThumbRX) > XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE) {
				gameObjects[0].transform.rotate.y += static_cast<float>(gamepadState.Gamepad.sThumbRX) / SHRT_MAX * rotationSpeed;
			}
			if (abs(gamepadState.Gamepad.sThumbRY) > XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE) {
				gameObjects[0].transform.rotate.x += static_cast<float>(gamepadState.Gamepad.sThumbRY) / SHRT_MAX * rotationSpeed;
			}
		}
		//g_debugCamera.Update(keys_, mouseState);
		// ImGuiウィンドウ
		ImGui::Begin("Settings");
		{
			ImGui::SeparatorText("Global Settings");
			const char* lightingItems[] = { "Lambert", "Half-Lambert", "None" };
			ImGui::Combo("Lighting Model", &selectedLightingOption, lightingItems, IM_ARRAYSIZE(lightingItems));

			if (selectedLightingOption == 2) {
				for (auto& gameObject : gameObjects) {
					if (gameObject.modelAssetIndex >= 0 && gameObject.modelAssetIndex < modelAssets.size()) {
						for (auto& mesh : modelAssets[gameObject.modelAssetIndex].modelData.meshes) {
							mesh.materialData->enableLighting = 0;
						}
					}
				}
			} else {
				for (auto& gameObject : gameObjects) {
					if (gameObject.modelAssetIndex >= 0 && gameObject.modelAssetIndex < modelAssets.size()) {
						for (auto& mesh : modelAssets[gameObject.modelAssetIndex].modelData.meshes) {
							mesh.materialData->enableLighting = 1;
						}
					}
				}
				lightingSettingsData->lightingModel = selectedLightingOption;
			}
			ImGui::ColorEdit4("Light Color", &directionalLightData->color.x);
			if (!gameObjects.empty() && gameObjects[0].modelAssetIndex >= 0 && gameObjects[0].modelAssetIndex < modelAssets.size() &&
				!modelAssets[gameObjects[0].modelAssetIndex].modelData.meshes.empty() &&
				modelAssets[gameObjects[0].modelAssetIndex].modelData.meshes[0].materialData->enableLighting != 0) {
				ImGui::SliderFloat3("Light Direction", &directionalLightData->direction.x, -1.0f, 1.0f);
				directionalLightData->direction = Normalize(directionalLightData->direction);
			} else {
				ImGui::Text("Light Direction: N/A (Lighting Disabled)");
			}
			ImGui::SeparatorText("Audio Settings");
			if (ImGui::Button("Play Alarm Sound")) {
				SoundPlayWave(xAudio2, alarmSound);
			}
			ImGui::SeparatorText("Sprite Settings");
			ImGui::Checkbox("Show Sprite", &isSpriteVisible);
			if (isSpriteVisible) {
				std::vector<const char*> textureNames;
				for (const auto& asset : textureAssets) { textureNames.push_back(asset.name.c_str()); }
				ImGui::Combo("Sprite Texture", &spriteTextureIndex, textureNames.data(), static_cast<int>(textureNames.size()));
				ImGui::DragFloat3("Sprite Pos", &transformSprite.translate.x, 1.0f);
				ImGui::DragFloat3("Sprite UV Scale", &uvTransformSprite.scale.x, 0.01f, 0.01f, 10.0f);
				ImGui::SliderAngle("Sprite UV Rotate Z", &uvTransformSprite.rotate.z);
				ImGui::DragFloat3("Sprite UV Translate", &uvTransformSprite.translate.x, 0.01f);
				float currentSpriteWidth = static_cast<float>(textureAssets[spriteTextureIndex].metadata.width);
				float currentSpriteHeight = static_cast<float>(textureAssets[spriteTextureIndex].metadata.height);
				vertexResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&vertexDataSprite));
				vertexDataSprite[0].position = { 0.0f, currentSpriteHeight, 0.0f, 1.0f };   vertexDataSprite[0].texcoord = { 0.0f, 1.0f };
				vertexDataSprite[1].position = { 0.0f, 0.0f, 0.0f, 1.0f };      vertexDataSprite[1].texcoord = { 0.0f, 0.0f };
				vertexDataSprite[2].position = { currentSpriteWidth, currentSpriteHeight, 0.0f, 1.0f };  vertexDataSprite[2].texcoord = { 1.0f, 1.0f };
				vertexDataSprite[3].position = { currentSpriteWidth, 0.0f, 0.0f, 1.0f };    vertexDataSprite[3].texcoord = { 1.0f, 0.0f };
				vertexResourceSprite->Unmap(0, nullptr);
			}
			ImGui::SeparatorText("Object Settings");
			for (int i = 0; i < gameObjects.size(); ++i) {
				GameObject& currentGameObject = gameObjects[i];
				ImGui::PushID(i);
				ImGui::SeparatorText(std::format("Object {}", i + 1).c_str());
				std::vector<const char*> modelNames;
				for (const auto& asset : modelAssets) { modelNames.push_back(asset.modelData.name.c_str()); }
				ImGui::Combo("Model", &currentGameObject.modelAssetIndex, modelNames.data(), static_cast<int>(modelNames.size()));
				if (currentGameObject.modelAssetIndex >= 0 && currentGameObject.modelAssetIndex < modelAssets.size() &&
					!modelAssets[currentGameObject.modelAssetIndex].modelData.meshes.empty() &&
					modelAssets[currentGameObject.modelAssetIndex].modelData.meshes[0].materialData) {
					Material* currentMaterial = modelAssets[currentGameObject.modelAssetIndex].modelData.meshes[0].materialData;
					ImGui::ColorEdit4("Material Color", &currentMaterial->color.x);
					Log(std::format("Current Model: {}, Material Color: R:{:.2f}, G:{:.2f}, B:{:.2f}, A:{:.2f}\n",
						modelAssets[currentGameObject.modelAssetIndex].modelData.name,
						currentMaterial->color.x, currentMaterial->color.y, currentMaterial->color.z, currentMaterial->color.w));
				}
				ImGui::DragFloat3("Position", &currentGameObject.transform.translate.x, 0.1f);
				ImGui::DragFloat3("Scale", &currentGameObject.transform.scale.x, 0.1f);
				ImGui::SliderAngle("Rotate X", &currentGameObject.transform.rotate.x);
				ImGui::SliderAngle("Rotate Y", &currentGameObject.transform.rotate.y);
				ImGui::SliderAngle("Rotate Z", &currentGameObject.transform.rotate.z);
				ImGui::PopID();
			}
		}
		ImGui::End();

		if (!gameObjects.empty() && gameObjects[0].modelAssetIndex >= 0 && gameObjects[0].modelAssetIndex < modelAssets.size() &&
			modelAssets[gameObjects[0].modelAssetIndex].modelData.name == "multiMesh.obj")
		{
			ImGui::Begin("Mesh Settings (Object 1)");
			{
				ModelData& currentModel = modelAssets[gameObjects[0].modelAssetIndex].modelData;
				if (!currentModel.meshes.empty()) {
					std::vector<const char*> meshNames;
					for (const auto& mesh : currentModel.meshes) { meshNames.push_back(mesh.name.c_str()); }
					if (selectedMeshIndex >= currentModel.meshes.size()) {
						selectedMeshIndex = 0;
					}
					ImGui::Combo("Select Mesh", &selectedMeshIndex, meshNames.data(), static_cast<int>(meshNames.size()));
					if (selectedMeshIndex >= 0 && selectedMeshIndex < currentModel.meshes.size()) {
						MeshObject& selectedMesh = currentModel.meshes[selectedMeshIndex];
						ImGui::SeparatorText(selectedMesh.name.c_str());
						ImGui::DragFloat3("Mesh Position", &selectedMesh.transform.translate.x, 0.1f);
						ImGui::DragFloat3("Mesh Scale", &selectedMesh.transform.scale.x, 0.1f);
						ImGui::SliderAngle("Mesh Rotate X", &selectedMesh.transform.rotate.x);
						ImGui::SliderAngle("Mesh Rotate Y", &selectedMesh.transform.rotate.y);
						ImGui::SliderAngle("Mesh Rotate Z", &selectedMesh.transform.rotate.z);
						ImGui::ColorEdit4("Mesh Color", &selectedMesh.materialData->color.x);
						if (selectedMesh.hasUV) {
							std::vector<const char*> textureNames;
							for (const auto& asset : textureAssets) { textureNames.push_back(asset.name.c_str()); }
							size_t meshTexIdx = selectedMesh.textureAssetIndex;
							ImGui::Combo("Mesh Texture", reinterpret_cast<int*>(&meshTexIdx), textureNames.data(), static_cast<int>(textureNames.size()));
							selectedMesh.textureAssetIndex = static_cast<int>(meshTexIdx);
							ImGui::SeparatorText("Mesh UV Transform");
							ImGui::DragFloat3("Mesh UV Scale", &selectedMesh.uvTransform.scale.x, 0.01f, 0.01f, 10.0f);
							ImGui::SliderAngle("Mesh UV Rotate Z", &selectedMesh.uvTransform.rotate.z);
							ImGui::DragFloat3("Mesh UV Translate", &selectedMesh.uvTransform.translate.x, 0.01f);
						} else {
							ImGui::Text("Mesh Texture: N/A (No UVs)");
							ImGui::Text("Mesh UV Transform: N/A (No UVs)");
						}
					}
				} else {
					ImGui::Text("No meshes in this model.");
				}
			}
			ImGui::End();
		}

		// 更新処理
		const Matrix4x4& viewMatrix = g_debugCamera.GetViewMatrix();
		Matrix4x4 projectionMatrix = MakePerspectiveMatrix(0.45f, float(winApp->kClientWidth) / float(winApp->kClientHeight), 0.1f, 100.0f);

		for (auto& gameObject : gameObjects) {
			Matrix4x4 globalWorldMatrix = MakeAffineMatrix(gameObject.transform.scale, gameObject.transform.rotate, gameObject.transform.translate);
			if (gameObject.modelAssetIndex >= 0 && gameObject.modelAssetIndex < modelAssets.size()) {
				ModelData& currentModel = modelAssets[gameObject.modelAssetIndex].modelData;
				for (auto& mesh : currentModel.meshes) {
					Matrix4x4 meshLocalWorldMatrix = MakeAffineMatrix(mesh.transform.scale, mesh.transform.rotate, mesh.transform.translate);
					Matrix4x4 finalWorldMatrix = Multiply(meshLocalWorldMatrix, globalWorldMatrix);
					mesh.wvpData->World = finalWorldMatrix;
					mesh.wvpData->WVP = Multiply(finalWorldMatrix, Multiply(viewMatrix, projectionMatrix));
					if (mesh.hasUV) {
						mesh.materialData->uvTransform = MakeAffineMatrix(mesh.uvTransform.scale, mesh.uvTransform.rotate, mesh.uvTransform.translate);
					} else {
						mesh.materialData->uvTransform = Identity4x4();
					}
				}
			}
		}

		if (isSpriteVisible) {
			Matrix4x4 worldMatrixSprite = MakeAffineMatrix(transformSprite.scale, transformSprite.rotate, transformSprite.translate);
			Matrix4x4 viewMatrixSprite = Identity4x4();
			Matrix4x4 projectionMatrixSprite = makeOrthographicmMatrix(0.0f, 0.0f, float(winApp->kClientWidth), float(winApp->kClientHeight), 0.0f, 100.0f);
			wvpDataSprite->World = worldMatrixSprite;
			wvpDataSprite->WVP = Multiply(worldMatrixSprite, Multiply(viewMatrixSprite, projectionMatrixSprite));
			materialDataSprite->color = { 1.0f, 1.0f, 1.0f, 1.0f };
			materialDataSprite->enableLighting = 0;
			materialDataSprite->shininess = 0.0f;
			materialDataSprite->uvTransform = MakeAffineMatrix(uvTransformSprite.scale, uvTransformSprite.rotate, uvTransformSprite.translate);
		}

		// --- 描画処理 ---
		// directXの描画前処理
		dxCommon->PreDraw();

		//spriteの描画前処理
		spriteCommon->SetupCommonState();

		// ここからアプリケーション固有の描画コマンド
		commandList->SetPipelineState(graphicsPipelineState.Get());
		commandList->SetGraphicsRootSignature(rootSignature.Get());
		commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		ID3D12DescriptorHeap* descriptorHeaps[] = { srvDescriptorHeap };
		commandList->SetDescriptorHeaps(1, descriptorHeaps);
		commandList->SetGraphicsRootConstantBufferView(3, directionalLightResource->GetGPUVirtualAddress());
		commandList->SetGraphicsRootConstantBufferView(4, lightingSettingsResource->GetGPUVirtualAddress());

		// 3Dオブジェクト描画
		for (auto& gameObject : gameObjects) {
			if (gameObject.modelAssetIndex >= 0 && gameObject.modelAssetIndex < modelAssets.size()) {
				ModelData& currentModel = modelAssets[gameObject.modelAssetIndex].modelData;
				for (auto& mesh : currentModel.meshes) {
					commandList->SetGraphicsRootConstantBufferView(0, mesh.materialResource->GetGPUVirtualAddress());
					commandList->SetGraphicsRootConstantBufferView(1, mesh.wvpResource->GetGPUVirtualAddress());
					commandList->IASetVertexBuffers(0, 1, &mesh.vertexBufferView);

					if (mesh.hasUV && !mesh.material.textureFilePath.empty()) {
						size_t meshTexIdx = 0;
						for (size_t i = 0; i < textureAssets.size(); ++i) {
							if (textureAssets[i].name == mesh.material.textureFilePath) {
								meshTexIdx = i;
								break;
							}
						}
						commandList->SetGraphicsRootDescriptorTable(2, textureAssets[meshTexIdx].gpuHandle);
					} else {
						commandList->SetGraphicsRootDescriptorTable(2, textureAssets[0].gpuHandle);
					}
					commandList->DrawInstanced(UINT(mesh.vertices.size()), 1, 0, 0);
				}
			}
		}

		// スプライト描画
		if (isSpriteVisible) {
			commandList->SetGraphicsRootConstantBufferView(0, materialResourceSprite->GetGPUVirtualAddress());
			commandList->SetGraphicsRootConstantBufferView(1, wvpResourceSprite->GetGPUVirtualAddress());
			commandList->IASetVertexBuffers(0, 1, &vertexBufferViewSprite);
			commandList->IASetIndexBuffer(&indexBufferViewSprite);
			commandList->SetGraphicsRootDescriptorTable(2, textureAssets[spriteTextureIndex].gpuHandle);
			commandList->DrawIndexedInstanced(6, 1, 0, 0, 0);
		}

		// ImGui描画
		ImGui::Render();
		ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), commandList);

		// 描画後処理
		dxCommon->PostDraw();
	}

	// クリーンアップ処理
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	SoundUnload(&alarmSound);
	if (masteringVoice) {
		masteringVoice->DestroyVoice();
	}
	if (xAudio2) {
		xAudio2->Release();
	}

	CloseHandle(dxCommon->GetFenceEvent());
	CoUninitialize();

	delete dxCommon;
	delete input;
	delete winApp;
	delete spriteCommon;
	delete sprite;

	return 0;
}