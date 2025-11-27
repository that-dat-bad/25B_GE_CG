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
#include"../engine/Graphics/TextureManager.h"
#include"../engine/Graphics/Object3dCommon.h"
#include"../engine/Graphics/Object3d.h"
#include"../engine/Graphics/ModelCommon.h"
#include"../engine/Graphics/Model.h"
#include"../engine/Graphics/ModelManager.h"
#include"../engine/base/Math/MyMath.h"
#include"../engine/Graphics/Camera.h"
#include"../engine/Graphics/CameraManager.h"
#include"../engine/Graphics/SrvManager.h"
#include "../engine/Debug/D3DResourceLeakChecker.h"
#include "../engine/Graphics/ParticleManager.h"
#include "../engine/Graphics/ParticleEmitter.h"
#include"../engine/Debug/ImguiManager.h"
using namespace MyMath;

// debug用のヘッダ
#include <DbgHelp.h>
#pragma comment(lib, "Dbghelp.lib")

#include <dxgidebug.h>
#pragma comment(lib, "dxguid.lib")

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

#pragma comment(lib,"dxcompiler.lib")




#include "../../external/DirectXTex/DirectXTex.h"
#include "../../external/DirectXTex/d3dx12.h"

#include <xaudio2.h>
#pragma comment(lib, "xaudio2.lib")

#include <Xinput.h>
#pragma comment(lib, "xinput.lib")

// 構造体の定義


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

	D3DResourceLeakChecker leakCheck;
	WinApp* winApp = new WinApp();
	winApp->Initialize();
	SetUnhandledExceptionFilter(ExportDump);

	Input* input = nullptr;
	input = new Input();
	input->Initialize(hInstance, winApp->GetHwnd());

	// DirectXCommonの初期化
	DirectXCommon* dxCommon = new DirectXCommon();
	dxCommon->Initialize(winApp);

	SrvManager* srvManager = nullptr;
	srvManager = new SrvManager();
	srvManager->Initialize(dxCommon);


	//ImguiManagerの初期化
	ImGuiManager* imguiManager = nullptr;
	imguiManager = new ImGuiManager();
	imguiManager->Initialize(winApp, dxCommon, srvManager);

	//TextureManagerの初期化
	TextureManager::GetInstance()->Initialize(dxCommon, srvManager);

	//Sprite共通部の初期化
	SpriteCommon* spriteCommon = new SpriteCommon();
	spriteCommon->Initialize(dxCommon);

	//Object3D共通部の初期化
	Object3dCommon* object3dCommon = new Object3dCommon();
	object3dCommon->Initialize(dxCommon);


	//3Dモデルマネージャーの初期化
	ModelManager::GetInstance()->Initialize(dxCommon);

	// DirectXCommonから必要なオブジェクトを取得
	ID3D12Device* device = dxCommon->GetDevice();
	ID3D12GraphicsCommandList* commandList = dxCommon->GetCommandList();
	ID3D12CommandQueue* commandQueue = dxCommon->GetCommandQueue();
	HRESULT hr = dxCommon->GetCommandAllocator()->Reset();
	assert(SUCCEEDED(hr));
	hr = commandList->Reset(dxCommon->GetCommandAllocator(), nullptr);
	assert(SUCCEEDED(hr));

	ParticleManager::GetInstance()->Initialize(dxCommon, srvManager);
	ParticleManager::GetInstance()->CreateParticleGroup("Circle", "assets/textures/checkerBoard.png");
	Transform emitterTransform;
	emitterTransform.translate = { 0.0f, 0.0f, -2.0f };
	emitterTransform.rotate = { 0.0f, 0.0f, 0.0f };
	emitterTransform.scale = { 1.0f, 1.0f, 1.0f };
	ParticleEmitter* particleEmitter = new ParticleEmitter("Circle", emitterTransform, 10, 0.5f);
#pragma endregion



	std::vector<std::string> texturePaths = {
		"assets/textures/uvchecker.png",
		"assets/textures/monsterBall.png",
		"assets/textures/checkerBoard.png",
	};


	for (const auto& path : texturePaths) {
		TextureManager::GetInstance()->LoadTexture(path);
	}

	// モデル読み込み
	std::vector<ModelAsset> modelAssets;
	std::vector<std::string> modelPaths = {
		//"sphere.obj",
		//"plane.obj",
	};


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
	sprite->Initialize(spriteCommon, dxCommon, "assets/textures/uvchecker.png");




	CameraManager::GetInstance()->Initialize();
	CameraManager::GetInstance()->CreateCamera("Global");
	CameraManager::GetInstance()->SetActiveCamera("Global");
	CameraManager::GetInstance()->GetActiveCamera()->SetTranslate({ 0, 20, -20 });
	CameraManager::GetInstance()->GetActiveCamera()->SetRotate({ 0.8f, 0, 0 });



	



	int currentSpriteIndex = 0;
	int spriteTextureIndex = 0;
	bool isSpriteVisible = false;
	bool isObjectVisible = true;

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
	const char* modes[] = { "None", "Normal", "Add", "Subtract", "Multiply", "Screen" };
	static int currentBlendMode = 1;
	while (true) {

		if (winApp->ProcessMessage()) {
			break;
		}

		imguiManager->Begin();

		input->Update();
		if (input->triggerKey(DIK_S)) {
			isObjectVisible = !isObjectVisible;
		}
		XINPUT_STATE gamepadState;
		ZeroMemory(&gamepadState, sizeof(XINPUT_STATE));
		DWORD dwResult = XInputGetState(0, &gamepadState);

		if (dwResult == ERROR_SUCCESS && !gameObjects.empty()) {
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

		CameraManager::GetInstance()->SetActiveCamera("Global");
		CameraManager::GetInstance()->Update();

		particleEmitter->Update();
		ParticleManager::GetInstance()->Update();
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
		ImGui::Begin("Sprite Settings");
		if (ImGui::Combo("Blend Mode", &currentBlendMode, modes, IM_ARRAYSIZE(modes))) {
			sprite->SetBlendMode(static_cast<BlendMode>(currentBlendMode));
		}

		ImGui::End();
		sprite->Update();


		// --- 描画処理 ---
		// directXの描画前処理
		dxCommon->PreDraw();
		srvManager->PreDraw();

		object3dCommon->SetupCommonState();
		commandList->SetGraphicsRootConstantBufferView(3, directionalLightResource->GetGPUVirtualAddress());
		commandList->SetGraphicsRootConstantBufferView(4, lightingSettingsResource->GetGPUVirtualAddress());
		ParticleManager::GetInstance()->Draw();
		// スプライト描画
		//spriteの描画前処理
		spriteCommon->SetupCommonState();
		//sprite->Draw(dxCommon, texturePaths[spriteTextureIndex].gpuHandle);
		sprite->Draw(dxCommon);


	// ImGui描画
		imguiManager->End();
		// 描画後処理
		dxCommon->PostDraw();
	}

	// クリーンアップ処理
	imguiManager->Finalize();
	delete imguiManager;

	SoundUnload(&alarmSound);
	if (masteringVoice) {
		masteringVoice->DestroyVoice();
	}
	if (xAudio2) {
		xAudio2->Release();
	}

	CloseHandle(dxCommon->GetFenceEvent());
	CoUninitialize();
	ModelManager::GetInstance()->Finalize();
	delete dxCommon;
	TextureManager::GetInstance()->Finalize();
	delete srvManager;
	delete particleEmitter;
	ParticleManager::GetInstance()->Finalize();
	delete input;
	delete winApp;
	delete spriteCommon;
	delete object3dCommon;


	return 0;
}