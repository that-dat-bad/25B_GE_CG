#include "Model.h"
#include"ModelCommon.h"
#include"DirectXCommon.h"
#include"TextureManager.h"
#include"ModelManager.h"
#include<cassert>
#include<map>
#include<fstream>
#include<sstream>
#include <filesystem>

void Model::LoadFromOBJ(const std::string& modelName) {

	ModelManager::GetInstance()->LoadModel(modelName);
}

Model* Model::CreateFromOBJ(const std::string& modelName, bool smoothing) {
	//マネージャ経由で読み込む
	ModelManager::GetInstance()->LoadModel(modelName);

	//読み込んだモデルのポインタを返す
	return ModelManager::GetInstance()->FindModel(modelName);
}


void Model::Initialize(ModelCommon* modelCommon, const std::string& directorypath, const std::string& filename) {

	modelCommon_ = modelCommon;
	DirectXCommon* dxCommon = modelCommon_->GetDirectXCommon();

	// 1. モデル読み込み
	modelData_ = LoadObjFile(directorypath, filename);

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

	// 6. テクスチャ読み込み

	if (!modelData_.material.textureFilePath.empty()) {
		TextureManager::GetInstance()->LoadTexture(modelData_.material.textureFilePath);
		modelData_.material.textureIndex = TextureManager::GetInstance()->GetTextureIndexByFilePath(modelData_.material.textureFilePath);
	} else {
		modelData_.material.textureIndex = 0;
	}

}

void Model::Draw() {

	// コマンドリストの取得
	ID3D12GraphicsCommandList* commandList = modelCommon_->GetDirectXCommon()->GetCommandList();

	// 頂点バッファの設定
	commandList->IASetVertexBuffers(0, 1, &vertexBufferView_);

	// マテリアルCBufferの設定 (RootParameter Index: 0)
	commandList->SetGraphicsRootConstantBufferView(0, materialResource_->GetGPUVirtualAddress());


	// テクスチャ (DescriptorTable) の設定 (RootParameter Index: 2)
	if (modelData_.material.textureIndex != 0) {
		D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandle = TextureManager::GetInstance()->GetSrvHandleGPU(modelData_.material.textureIndex);
		commandList->SetGraphicsRootDescriptorTable(2, textureSrvHandle);
	}

	// 描画コマンド発行
	commandList->DrawInstanced(UINT(modelData_.vertices.size()), 1, 0, 0);

}

Model::MaterialData Model::LoadMaterialTemplate(const std::string& directoryPath, const std::string& filename) {
	MaterialData materialData;
	materialData.textureIndex = 0;

	// パスを構築 (generic_stringで区切り文字を統一)
	std::filesystem::path dir(directoryPath);
	std::filesystem::path file(filename);
	std::string fullPath = (dir / file).generic_string();

	std::ifstream mtlFile(fullPath);

	// ファイルが開けなかった場合のエラーチェックを追加
	if (!mtlFile.is_open()) {
		std::string msg = "Failed to open .mtl file!\nPath: " + fullPath;
		MessageBoxA(nullptr, msg.c_str(), "Model Error", MB_OK | MB_ICONERROR);
		assert(false);
		return materialData;
	}

	std::string line;
	while (std::getline(mtlFile, line)) {
		std::istringstream s(line);
		std::string identifier;
		s >> identifier;

		// テクスチャファイル名 (map_Kd)
		if (identifier == "map_Kd") {
			std::string textureFileName;
			s >> textureFileName;

			// ディレクトリパスと結合してテクスチャのフルパスを作る
			std::string texturePath = (dir / textureFileName).generic_string();

			// テクスチャパスを保存
			materialData.textureFilePath = texturePath;
		}
	}
	return materialData;
}

Model::ModelData Model::LoadObjFile(const std::string& directoryPath, const std::string& filename) {
	ModelData modelData; // 返却用
	std::vector<Vector4> positions;
	std::vector<Vector3> normals;
	std::vector<Vector2> texcoords;
	std::string line;

	// ★修正: std::filesystem を使ってパスを綺麗に結合する
	// これで "assets" + "/" + "models\\axis.obj" みたいな変なパスになるのを防ぎます
	std::filesystem::path dir(directoryPath);
	std::filesystem::path file(filename);
	std::string fullPath = (dir / file).generic_string();

	// ファイルを開く (変数名を objFile に変更して衝突回避)
	std::ifstream objFile(fullPath);
	assert(objFile.is_open());

	// .mtlファイルを読み込むための基準ディレクトリを計算
	std::string baseDirectory;
	size_t pos = fullPath.find_last_of('/');
	if (pos == std::string::npos) {
		pos = fullPath.find_last_of('\\'); // Windowsの区切り文字(\)も考慮
	}

	if (pos != std::string::npos) {
		baseDirectory = fullPath.substr(0, pos);
	} else {
		baseDirectory = "";
	}

	// 行ごとの読み込み
	while (std::getline(objFile, line)) {
		std::string identifier;
		std::istringstream s(line);
		s >> identifier;

		if (identifier == "v") {
			// 頂点位置
			Vector4 position;
			s >> position.x >> position.y >> position.z;
			position.w = 1.0f;
			position.x *= -1.0f; // 右手系(Blender等) -> 左手系(DirectX)への変換
			positions.push_back(position);
		} else if (identifier == "vt") {
			// テクスチャ座標
			Vector2 texcoord;
			s >> texcoord.x >> texcoord.y;
			texcoord.y = 1.0f - texcoord.y; // V方向反転
			texcoords.push_back(texcoord);
		} else if (identifier == "vn") {
			// 法線ベクトル
			Vector3 normal;
			s >> normal.x >> normal.y >> normal.z;
			normal.x *= -1.0f; // 右手系 -> 左手系
			normals.push_back(normal);
		} else if (identifier == "f") {
			// 面情報
			VertexData triangle[3];
			for (int32_t faceVertex = 0; faceVertex < 3; ++faceVertex) {
				std::string vertexDefinition;
				s >> vertexDefinition;

				std::istringstream v(vertexDefinition);
				std::string indexStr;
				int indices[3] = { 0, 0, 0 }; // 位置, UV, 法線
				int i = 0;

				// スラッシュ区切りでインデックスを読む (例: 1/1/1)
				while (std::getline(v, indexStr, '/')) {
					if (!indexStr.empty()) {
						indices[i] = std::stoi(indexStr);
					}
					i++;
				}

				// インデックスを使って頂点を構築 (OBJは1始まりなので -1 する)
				triangle[faceVertex].position = positions[indices[0] - 1];

				if (indices[1] != 0) triangle[faceVertex].texcoord = texcoords[indices[1] - 1];
				else triangle[faceVertex].texcoord = { 0.0f, 0.0f };

				if (indices[2] != 0) triangle[faceVertex].normal = normals[indices[2] - 1];
				else triangle[faceVertex].normal = { 0.0f, 0.0f, 1.0f };
			}

			// 頂点を逆順で登録 (0, 2, 1) することでカリング対策
			modelData.vertices.push_back(triangle[0]);
			modelData.vertices.push_back(triangle[2]);
			modelData.vertices.push_back(triangle[1]);
		} else if (identifier == "mtllib") {
			// マテリアルファイル読み込み
			std::string materialFileName;
			s >> materialFileName;
			modelData.material = LoadMaterialTemplate(baseDirectory, materialFileName);
		}
	}

	return modelData;
}