#include "Object3d.h"
#include "Object3dCommon.h"
#include<map>
#include<fstream>
#include<sstream>
#include<cassert>
#include"TextureManager.h"

void Object3d::Initialize(Object3dCommon* object3dCommon)
{
	object3dCommon_ = object3dCommon;
	//モデル読み込み
	modelData_ = LoadObjFile("assets/models", "plane.obj");

	//vertexResource(vertexBuffer)を作る
	vertexBuffer_ = dxCommon->CreateBufferResource(sizeof(VertexData) * 4);
	//VertexBufferViewの作成
	vertexBufferView_.BufferLocation = vertexBuffer_->GetGPUVirtualAddress();
	vertexBufferView_.SizeInBytes = sizeof(VertexData) * 4;
	vertexBufferView_.StrideInBytes = sizeof(VertexData);
	//VertexResourceにデータを書き込むためのアドレスを取得してvertexDataに割り当てる
	vertexBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData_));
	//マテリアルリソースの作成
	materialResource_ = dxCommon->CreateBufferResource(sizeof(Material));
	//MaterialDataの割り当て
	materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&materialData_));
	//マテリアルデータの初期化
	materialData_->color = { 1.0f,1.0f,1.0f,1.0f };
	materialData_->enableLighting = false;
	materialData_->uvTransform = Identity4x4();
	//座標変換行列リソースの作成
	transformationMatrixResource_ = dxCommon->CreateBufferResource(sizeof(TransformationMatrix));
	//TransformationMatrixの割り当て
	transformationMatrixResource_->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixData_));

	transformationMatrixData_->WVP = Identity4x4();
	transformationMatrixData_->World = Identity4x4();

	//平行光源リソースの作成
	directionalLightResource_ = dxCommon->CreateBufferResource(sizeof(DirectionalLight));
	//DirectionalLightの割り当て
	directionalLightResource_->Map(0, nullptr, reinterpret_cast<void**>(&directionalLightData_));
	directionalLightData_->color = { 1.0f,1.0f,1.0f,1.0f };
	directionalLightData_->direction = { 0.0f,-1.0f,0.0f };
	directionalLightData_->intensity = 1.0f;
	//.objの参照しているテクスチャファイル読み込み
	TextureManager::GetInstance()->LoadTexture(modelData_.material.textureFilePath);
	//読み込んだテクスチャの番号を取得
	TextureManager::GetInstance()->GetTextureIndexByFilePath(modelData_.material.textureFilePath);

	transform_ = { {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} };
	cameraTransform_ = { {1.0f,1.0f,1.0f},{0.3f,0.0f,0.0f},{0.0f,0.4f,-10.0f} };

}

Object3d::MaterialData Object3d::LoadMaterialTemplate(const std::string& directoryPath, const std::string& filename)
{
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

Object3d::ModelData Object3d::LoadObjFile(const std::string& directoryPath, const std::string& filename)
{
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
