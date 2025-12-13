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
#include"../engine/Audio/AudioManager.h"
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
	TextureManager::GetInstance()->Initialize(dxCommon,srvManager);

	//Sprite共通部の初期化
	SpriteCommon* spriteCommon = new SpriteCommon();
	spriteCommon->Initialize(dxCommon);

	//Object3D共通部の初期化
	Object3dCommon* object3dCommon = new Object3dCommon();
	object3dCommon->Initialize(dxCommon);

	AudioManager* audioManager = new AudioManager();
	audioManager->Initialize();

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

	int selectedMeshIndex = 0;

	// スプライトの初期化

	std::vector<Sprite*> sprites;
	const int kSpriteCount = 5; // 5枚描画してみる

	for (int i = 0; i < kSpriteCount; ++i) {
		Sprite* newSprite = new Sprite();
		if (i == 0 || i == 2 || i == 4) { // 1, 3, 5枚目 (インデックス 0, 2, 4)
			newSprite->Initialize(spriteCommon, dxCommon, "assets/textures/monsterBall.png");

		} else { // 2, 4枚目 (インデックス 1, 3)
			newSprite->Initialize(spriteCommon, dxCommon, "assets/textures/uvchecker.png");

		}
		//newSprite->Initialize(spriteCommon, dxCommon, "assets/textures/monsterBall.png");

		// 横に並ぶように初期位置をずらす
		Vector2 pos = { 100.0f + (i * 150.0f), 200.0f };
		newSprite->SetPosition(pos);

		sprites.push_back(newSprite);
	}


	CameraManager::GetInstance()->Initialize();
	CameraManager::GetInstance()->CreateCamera("Global"); // 俯瞰用
	CameraManager::GetInstance()->CreateCamera("Player"); // プレイヤー視点用
	CameraManager::GetInstance()->SetActiveCamera("Global");
	CameraManager::GetInstance()->GetActiveCamera()->SetTranslate({ 0, 20, -20 });
	CameraManager::GetInstance()->GetActiveCamera()->SetRotate({ 0.8f, 0, 0 });

	CameraManager::GetInstance()->SetActiveCamera("Player");
	CameraManager::GetInstance()->GetActiveCamera()->SetTranslate({ 0, 0, -5 });

	Model* model = new Model();
	ModelManager::GetInstance()->LoadModel("models/axis.obj");
	ModelManager::GetInstance()->LoadModel("models/teapot.obj");
	Object3d* objectAxis = new Object3d();
	objectAxis->Initialize(object3dCommon);
	Object3d* objectPlane = new Object3d();
	objectPlane->Initialize(object3dCommon);
	Model* modelAxis = ModelManager::GetInstance()->FindModel("models/axis.obj");
	Model*modelPlane= ModelManager::GetInstance()->FindModel("models/teapot.obj");
	objectAxis->SetModel(modelAxis);
	objectAxis->SetScale({ 1.0f, 1.0f, 1.0f });
	objectPlane->SetModel(modelPlane);
	objectPlane->SetScale({ 1.0f, 1.0f, 1.0f });



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
	SoundData alarmSound =audioManager->SoundLoadWave("assets/sounds/Alarm01.wav");

	int selectedLightingOption = 0;

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


		//g_debugCamera.Update(keys_, mouseState);
		// ImGuiウィンドウ
		if (input->triggerKey(DIK_SPACE)) {
			static bool isGlobal = false;
			isGlobal = !isGlobal;
			if (isGlobal) {
				CameraManager::GetInstance()->SetActiveCamera("Global");
			} else {
				CameraManager::GetInstance()->SetActiveCamera("Player");
			}
		}
#ifdef USE_IMGUI



		ImGui::Begin("Settings");
		{
			ImGui::SeparatorText("Global Settings");
			ImGui::SeparatorText("Audio Settings");
			if (ImGui::Button("Play Alarm Sound")) {
				audioManager->SoundPlayWave(alarmSound);
			}
			ImGui::SeparatorText("Sprite Settings");
			ImGui::Checkbox("Show Sprite", &isSpriteVisible);
			if (isSpriteVisible) {
				// 1. テクスチャ選択 (共通)
				std::vector<const char*> textureNames;
				for (const auto& path : texturePaths) { textureNames.push_back(path.c_str()); }
				ImGui::Combo("Sprite Texture", &spriteTextureIndex, textureNames.data(), static_cast<int>(textureNames.size()));

				ImGui::Separator();

				//操作するスプライトを選択
				ImGui::SliderInt("Select Sprite No", &currentSpriteIndex, 0, int(sprites.size()) - 1);

				// 選ばれたスプライトを取得
				Sprite* targetSprite = sprites[currentSpriteIndex];

				ImGui::Text("Editing Sprite: %d", currentSpriteIndex);

				// --------------------------------------------------
				// 2. 座標 (Translate)
				// --------------------------------------------------
				Vector2 pos = targetSprite->GetPosition();
				if (ImGui::DragFloat2("Position", &pos.x, 1.0f)) {
					targetSprite->SetPosition(pos);
				}

				// --------------------------------------------------
				// 3. 回転 (Rotate)
				// --------------------------------------------------
				float rot = targetSprite->GetRotation();
				if (ImGui::SliderAngle("Rotation", &rot)) {
					targetSprite->SetRotation(rot);
				}

				// --------------------------------------------------
				// 4. サイズ (Scale)
				// --------------------------------------------------
				Vector2 size = targetSprite->GetSize();
				if (ImGui::DragFloat2("Size", &size.x, 1.0f)) {
					targetSprite->SetSize(size);
				}

				// --------------------------------------------------
				// 5. 色 (Color)
				// --------------------------------------------------
				Vector4 color = targetSprite->GetColor();
				if (ImGui::ColorEdit4("Color", &color.x)) {
					targetSprite->SetColor(color);
				}
			}
		}
		ImGui::End();

		
#endif // USE_IMGUI
		CameraManager::GetInstance()->Update();
		objectAxis->SetCamera(CameraManager::GetInstance()->GetActiveCamera());
		objectPlane->SetCamera(CameraManager::GetInstance()->GetActiveCamera());
		objectAxis->Update();
		objectPlane->Update();
		if (input->triggerKey(DIK_A)) {
			particleEmitter->Emit();
		}
		particleEmitter->Update();
		ParticleManager::GetInstance()->Update();
		// 更新処理
		const Matrix4x4& viewMatrix = g_debugCamera.GetViewMatrix();
		Matrix4x4 projectionMatrix = MakePerspectiveMatrix(0.45f, float(winApp->kClientWidth) / float(winApp->kClientHeight), 0.1f, 100.0f);

		if (isSpriteVisible) {
			/*sprite->Update();*/
			for (Sprite* sprite : sprites) {
				sprite->Update();
			}
		}

		// --- 描画処理 ---
		// directXの描画前処理
		dxCommon->PreDraw();
		srvManager->PreDraw();






		object3dCommon->SetupCommonState();
		commandList->SetGraphicsRootConstantBufferView(3, directionalLightResource->GetGPUVirtualAddress());
		commandList->SetGraphicsRootConstantBufferView(4, lightingSettingsResource->GetGPUVirtualAddress());
		if (isObjectVisible) {
			objectAxis->Draw();
			objectPlane->Draw();
		}
		ParticleManager::GetInstance()->Draw();
		// スプライト描画
		if (isSpriteVisible) {
			//spriteの描画前処理
			spriteCommon->SetupCommonState();
			//sprite->Draw(dxCommon, texturePaths[spriteTextureIndex].gpuHandle);
			for (Sprite* sprite : sprites) {
				// 文字列配列からパスを取り出す
				std::string path = texturePaths[spriteTextureIndex];

				// TextureManagerからハンドルを取得
				D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandle =
					TextureManager::GetInstance()->GetSrvHandleGPU(spriteTextureIndex);

				// 描画
				sprite->Draw(dxCommon, textureSrvHandle);
			}
		}



		// ImGui描画
		imguiManager->End();
		// 描画後処理
		dxCommon->PostDraw();
	}

	// クリーンアップ処理
	imguiManager->Finalize();
	delete imguiManager;

	audioManager->SoundUnload(&alarmSound);
	if (masteringVoice) {
		masteringVoice->DestroyVoice();
	}
	if (xAudio2) {
		xAudio2->Release();
	}

	CloseHandle(dxCommon->GetFenceEvent());
	CoUninitialize();
	ModelManager::GetInstance()->Finalize();

	TextureManager::GetInstance()->Finalize();
	delete srvManager;
	delete particleEmitter;
	ParticleManager::GetInstance()->Finalize();
	delete input;

	delete spriteCommon;
	audioManager->SoundUnload(&alarmSound);

	delete audioManager;
	delete objectAxis;
	delete objectPlane;
	delete model;

	for (Sprite* sprite : sprites) {
		delete sprite;
	}
	delete object3dCommon;
	delete dxCommon;
	delete winApp;


	return 0;
}