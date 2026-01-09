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

// 構造体の定義など（省略せずそのまま残しています）
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
	Matrix4x4 uvTransform;
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
	int32_t lightingModel;
	float padding[3];
};

struct MaterialData {
	std::string textureFilePath;
	std::string name;
};

struct MeshObject {
	std::string name;
	std::vector<VertexData> vertices;
	MaterialData material;
	Transform transform;
	Transform uvTransform;
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource;
	Material* materialData = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> wvpResource;
	TransformationMatrix* wvpData = nullptr;
	int textureAssetIndex = 0;
	bool hasUV = false;
};

struct ModelData {
	std::string name;
	std::vector<MeshObject> meshes;
};

struct TextureAsset {
	std::string name;
	Microsoft::WRL::ComPtr<ID3D12Resource> resource;
	D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle;
	DirectX::TexMetadata metadata;
};

struct ModelAsset {
	ModelData modelData;
};

struct GameObject {
	Transform transform;
	int modelAssetIndex = 0;
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
	// (省略せずそのまま)
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

	// パーティクルマネージャ初期化
	ParticleManager::GetInstance()->Initialize(dxCommon, srvManager);

	// --- 炎の設定 ---
	ParticleManager::GetInstance()->CreateParticleGroup("Fire", "assets/textures/whiteCircle128_84.png");
	ParticleManager::GetInstance()->SetBlendMode("Fire", BlendMode::kAdd);
	Transform emitterTransform;
	emitterTransform.translate = { 0.0f, 0.0f, -2.0f };
	emitterTransform.rotate = { 0.0f, 0.0f, 0.0f };
	emitterTransform.scale = { 1.0f, 1.0f, 1.0f };

	Vector3 defaultVelocity = { 0.0f, 0.1f, 0.0f }; // デフォルトの上昇速度

	// 炎エミッター作成
	ParticleEmitter* fireEmitter = new ParticleEmitter(
		"Fire",
		emitterTransform,
		10,                         // 一度に出す数
		0.1f,                       // 0.1秒ごとに発生
		{ 1.0f, 0.8f, 0.2f, 1.0f }, // 黄色っぽい明るい色
		defaultVelocity,            // 上昇速度
		0.05f                       // 拡散
	);

	// --- マウス火花の設定 ---
	ParticleManager::GetInstance()->CreateParticleGroup("Spark", "assets/textures/whiteCircle128_84.png");
	ParticleManager::GetInstance()->SetBlendMode("Spark", BlendMode::kAdd);

#pragma endregion

	std::vector<std::string> texturePaths = {
		"assets/textures/uvchecker.png",
		"assets/textures/monsterBall.png",
		"assets/textures/checkerBoard.png",
		"assets/textures/black_1920x1080.png"
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

	int selectedMeshIndex = 0;

	// スプライトの初期化
	Sprite* sprite = new Sprite();
	sprite->Initialize(spriteCommon, dxCommon, "assets/textures/black_1920x1080.png");
	sprite->SetAnchorPoint({ 0.5f,0.5f });
	sprite->SetPosition({ float(winApp->kClientWidth) / 2.0f,float(winApp->kClientHeight) / 2.0f });

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

	// マウス位置保存用
	Vector3 prevMousePos = { 0, 0, 0 };

	while (true) {

		if (winApp->ProcessMessage()) {
			break;
		}

		imguiManager->Begin();

		input->Update(); // 入力更新

		XINPUT_STATE gamepadState;
		ZeroMemory(&gamepadState, sizeof(XINPUT_STATE));
		DWORD dwResult = XInputGetState(0, &gamepadState);

		// ★風の処理（アプリケーション層で実装）
		if (input->pushKey(DIK_SPACE)) {
			// スペースを押している間、左向きの成分を加える
			Vector3 windyVelocity = { -0.1f, 0.1f, 0.0f };
			fireEmitter->SetVelocity(windyVelocity);
		}
		else {
			// 離したら元の速度に戻す
			fireEmitter->SetVelocity(defaultVelocity);
		}


		// ★マウスの火花処理
		// マウス座標取得
		POINT mousePoint;
		GetCursorPos(&mousePoint);
		ScreenToClient(winApp->GetHwnd(), &mousePoint);

		// 3D座標変換 (簡易版)
		float mouseX = (float(mousePoint.x) - float(winApp->kClientWidth) / 2.0f) / 100.0f;
		float mouseY = -(float(mousePoint.y) - float(winApp->kClientHeight) / 2.0f) / 100.0f;
		Vector3 currentMousePos = { mouseX, mouseY, -2.0f };

		// 左クリック(0)判定
		if (input->PushMouse(0)) {
			ParticleManager::GetInstance()->Emit(
				"Spark",
				currentMousePos,
				prevMousePos,
				5,
				{ 1.0f, 0.1f, 0.1f, 1.0f }, // 赤色
				{ 0.0f, 0.0f, 0.0f },
				0.1f
			);
		}
		prevMousePos = currentMousePos;


		//g_debugCamera.Update(keys_, mouseState);
		// ImGuiウィンドウ

		CameraManager::GetInstance()->SetActiveCamera("Global");
		CameraManager::GetInstance()->Update();

		// エミッター更新 (風処理適用後)
		fireEmitter->Update();

		ParticleManager::GetInstance()->Update();
		// 更新処理
		const Matrix4x4& viewMatrix = g_debugCamera.GetViewMatrix();
		Matrix4x4 projectionMatrix = MakePerspectiveMatrix(0.45f, float(winApp->kClientWidth) / float(winApp->kClientHeight), 0.1f, 100.0f);
#ifdef USE_IMGUI

		ImGui::Begin("Sprite Settings");
		if (ImGui::Combo("Blend Mode", &currentBlendMode, modes, IM_ARRAYSIZE(modes))) {
			sprite->SetBlendMode(static_cast<BlendMode>(currentBlendMode));
		}

		ImGui::End();
#endif // USE_IMGUI
		sprite->Update();


		// --- 描画処理 ---
		// directXの描画前処理
		dxCommon->PreDraw();
		srvManager->PreDraw();

		object3dCommon->SetupCommonState();
		commandList->SetGraphicsRootConstantBufferView(3, directionalLightResource->GetGPUVirtualAddress());
		commandList->SetGraphicsRootConstantBufferView(4, lightingSettingsResource->GetGPUVirtualAddress());

		// スプライト描画
		//spriteの描画前処理
		spriteCommon->SetupCommonState();
		sprite->Draw(dxCommon);

		// パーティクル描画 (スプライトの後 = 最前面)
		ParticleManager::GetInstance()->Draw();

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

	// 1. ゲーム内オブジェクト削除
	delete fireEmitter;

	// 2. マネージャー/Common系のFinalizeと削除
	ModelManager::GetInstance()->Finalize();
	TextureManager::GetInstance()->Finalize();
	ParticleManager::GetInstance()->Finalize();

	delete spriteCommon;
	delete object3dCommon;
	delete srvManager;

	// 3. DirectX系削除
	delete dxCommon;

	// 4. 入力/Window系削除
	delete input;
	delete winApp;

	return 0;
}