#include "Object3d.h"
#include "Object3dCommon.h"
#include "TextureManager.h" // パスに合わせて調整してください
#include <map>
#include <fstream>
#include <sstream>
#include <cassert>


void Object3d::Initialize(Object3dCommon* object3dCommon)
{
	// メンバ変数に保存
	object3dCommon_ = object3dCommon;


	DirectXCommon* dxCommon = object3dCommon_->GetDirectXCommon();

	// 1. モデル読み込み
	modelData_ = LoadObjFile("assets/models", "axis.obj");

	// 2. 頂点データの初期化 (GPUリソース作成)

	// バッファリソース作成 (サイズは読み込んだ頂点数に合わせる)
	vertexBuffer_ = dxCommon->CreateBufferResource(sizeof(VertexData) * modelData_.vertices.size());

	// VertexBufferViewの作成
	vertexBufferView_.BufferLocation = vertexBuffer_->GetGPUVirtualAddress();
	vertexBufferView_.SizeInBytes = UINT(sizeof(VertexData) * modelData_.vertices.size());
	vertexBufferView_.StrideInBytes = sizeof(VertexData);

	// データの書き込み
	vertexBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData_));
	std::memcpy(vertexData_, modelData_.vertices.data(), sizeof(VertexData) * modelData_.vertices.size());
	vertexBuffer_->Unmap(0, nullptr);



	// 3. マテリアルの初期化

	materialResource_ = dxCommon->CreateBufferResource(sizeof(Material));
	materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&materialData_));

	materialData_->color = { 1.0f, 1.0f, 1.0f, 1.0f };
	materialData_->enableLighting = false;
	materialData_->shininess = 50.0f;
	materialData_->uvTransform = Identity4x4();



	// 4. 座標変換行列の初期化

	transformationMatrixResource_ = dxCommon->CreateBufferResource(sizeof(TransformationMatrix));
	transformationMatrixResource_->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixData_));

	transformationMatrixData_->WVP = Identity4x4();
	transformationMatrixData_->World = Identity4x4();



	// 5. 平行光源の初期化

	directionalLightResource_ = dxCommon->CreateBufferResource(sizeof(DirectionalLight));
	directionalLightResource_->Map(0, nullptr, reinterpret_cast<void**>(&directionalLightData_));

	directionalLightData_->color = { 1.0f, 1.0f, 1.0f, 1.0f };
	directionalLightData_->direction = { 0.0f, -1.0f, 0.0f };
	directionalLightData_->intensity = 1.0f;

	// 6. テクスチャ読み込み

	if (!modelData_.material.textureFilePath.empty()) {
		TextureManager::GetInstance()->LoadTexture(modelData_.material.textureFilePath);
		modelData_.material.textureIndex = TextureManager::GetInstance()->GetTextureIndexByFilePath(modelData_.material.textureFilePath);
	}
	else {
		modelData_.material.textureIndex = 0;
	}

	// 7. Transform初期値設定

	transform_ = { {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f} };
	cameraTransform_ = { {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, -10.0f} };
}

void Object3d::Update()
{
	// TransformからWorldMatrixを作る
	Matrix4x4 worldMatrix = MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.translate);

	// CameraTransformからCameraMatrixを作る
	Matrix4x4 cameraMatrix = MakeAffineMatrix(cameraTransform_.scale, cameraTransform_.rotate, cameraTransform_.translate);

	// CameraMatrixからViewMatrixを作る (逆行列)
	Matrix4x4 viewMatrix = Inverse(cameraMatrix);
	//Matrix4x4 viewMatrix = MakeTranslateMatrix({ 0.0f, 0.0f, 10.0f });
	// ProjectionMatrixを作る
	Matrix4x4 projectionMatrix = MakePerspectiveMatrix(0.45f, 1280.0f / 720.0f, 0.1f, 100.0f);

	// WVP行列を作成して書き込む
	Matrix4x4 worldViewProjectionMatrix = Multiply(worldMatrix, Multiply(viewMatrix, projectionMatrix));

	transformationMatrixData_->WVP = worldViewProjectionMatrix;
	transformationMatrixData_->World = worldMatrix;
}

void Object3d::Draw()
{
	// コマンドリストの取得
	ID3D12GraphicsCommandList* commandList = object3dCommon_->GetDirectXCommon()->GetCommandList();

	// 頂点バッファの設定
	commandList->IASetVertexBuffers(0, 1, &vertexBufferView_);

	// マテリアルCBufferの設定 (RootParameter Index: 0)
	commandList->SetGraphicsRootConstantBufferView(0, materialResource_->GetGPUVirtualAddress());

	// 座標変換行列CBufferの設定 (RootParameter Index: 1)
	commandList->SetGraphicsRootConstantBufferView(1, transformationMatrixResource_->GetGPUVirtualAddress());

	// テクスチャ (DescriptorTable) の設定 (RootParameter Index: 2)
	if (modelData_.material.textureIndex != 0) {
		D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandle = TextureManager::GetInstance()->GetSrvHandleGPU(modelData_.material.textureIndex);
		commandList->SetGraphicsRootDescriptorTable(2, textureSrvHandle);
	}

	// 平行光源CBufferの設定 (RootParameter Index: 3)
	commandList->SetGraphicsRootConstantBufferView(3, directionalLightResource_->GetGPUVirtualAddress());

	// 描画コマンド発行
	commandList->DrawInstanced(UINT(modelData_.vertices.size()), 1, 0, 0);
}

Object3d::MaterialData Object3d::LoadMaterialTemplate(const std::string& directoryPath, const std::string& filename)
{
	MaterialData materialData; // 返却用
	materialData.textureIndex = 0; // 初期化

	std::map<std::string, std::string> materials; // 名前管理用
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
		}
		else if (identifier == "map_Kd") {
			std::string textureFileName;
			s >> textureFileName;
			// テクスチャパスを保存
			materialData.textureFilePath = directoryPath + "/" + textureFileName;
		}
	}
	return materialData;
}

Object3d::ModelData Object3d::LoadObjFile(const std::string& directoryPath, const std::string& filename)
{
	ModelData modelData; // 返却用
	std::vector<Vector4> positions;
	std::vector<Vector3> normals;
	std::vector<Vector2> texcoords;
	std::string line;
	std::ifstream file(directoryPath + "/" + filename);
	assert(file.is_open());

	while (std::getline(file, line)) {
		std::string identifier;
		std::istringstream s(line);
		s >> identifier;

		if (identifier == "v") {
			Vector4 position;
			s >> position.x >> position.y >> position.z;
			position.w = 1.0f;
			position.x *= -1.0f; // RH->LH
			positions.push_back(position);
		}
		else if (identifier == "vt") {
			Vector2 texcoord;
			s >> texcoord.x >> texcoord.y;
			texcoord.y = 1.0f - texcoord.y; // V-flip
			texcoords.push_back(texcoord);
		}
		else if (identifier == "vn") {
			Vector3 normal;
			s >> normal.x >> normal.y >> normal.z;
			normal.x *= -1.0f; // RH->LH
			normals.push_back(normal);
		}
		else if (identifier == "f") {
			// 面データの読み込み
			VertexData triangle[3];
			for (int32_t faceVertex = 0; faceVertex < 3; ++faceVertex) {
				std::string vertexDefinition;
				s >> vertexDefinition;

				std::istringstream v(vertexDefinition);
				std::string indexStr;
				int indices[3] = { 0, 0, 0 };
				int i = 0;

				while (std::getline(v, indexStr, '/')) {
					if (!indexStr.empty()) {
						indices[i] = std::stoi(indexStr);
					}
					i++;
				}

				// 頂点構築
				triangle[faceVertex].position = positions[indices[0] - 1];

				if (indices[1] != 0) triangle[faceVertex].texcoord = texcoords[indices[1] - 1];
				else triangle[faceVertex].texcoord = { 0.0f, 0.0f };

				if (indices[2] != 0) triangle[faceVertex].normal = normals[indices[2] - 1];
				else triangle[faceVertex].normal = { 0.0f, 0.0f, 1.0f };
			}
			// 逆順登録 (カリング対策)
			modelData.vertices.push_back(triangle[0]);
			modelData.vertices.push_back(triangle[2]);
			modelData.vertices.push_back(triangle[1]);
		}
		else if (identifier == "mtllib") {
			std::string materialFileName;
			s >> materialFileName;
			// マテリアル読み込み
			modelData.material = LoadMaterialTemplate(directoryPath, materialFileName);
		}
	}

	// ここではGPUリソースを作らず、読み込んだデータを返すだけにする
	return modelData;
}