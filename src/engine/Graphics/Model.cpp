#include "Model.h"
#include"ModelCommon.h"
#include"DirectXCommon.h"
#include"TextureManager.h"
#include<cassert>
#include<map>
#include<fstream>
#include<sstream>
void Model::Initialize(ModelCommon* modelCommon, const std::string& directorypath, const std::string& filename)
{

	modelCommon_ = modelCommon;
	DirectXCommon* dxCommon = modelCommon_->GetDirectXCommon();

	modelData_ = LoadObjFile(directorypath, filename);


	vertexBuffer_ = dxCommon->CreateBufferResource(sizeof(VertexData) * modelData_.vertices.size());

	vertexBufferView_.BufferLocation = vertexBuffer_->GetGPUVirtualAddress();
	vertexBufferView_.SizeInBytes = UINT(sizeof(VertexData) * modelData_.vertices.size());
	vertexBufferView_.StrideInBytes = sizeof(VertexData);

	vertexBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData_));
	std::memcpy(vertexData_, modelData_.vertices.data(), sizeof(VertexData) * modelData_.vertices.size());
	vertexBuffer_->Unmap(0, nullptr);


	materialResource_ = dxCommon->CreateBufferResource(sizeof(Material));
	materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&materialData_));

	materialData_->color = { 1.0f, 1.0f, 1.0f, 1.0f };
	materialData_->enableLighting = true;
	materialData_->shininess = 50.0f;
	materialData_->uvTransform = Identity4x4();


	if (!modelData_.material.textureFilePath.empty()) {
		TextureManager::GetInstance()->LoadTexture(modelData_.material.textureFilePath);
		modelData_.material.textureIndex = TextureManager::GetInstance()->GetTextureIndexByFilePath(modelData_.material.textureFilePath);
	} else {
		modelData_.material.textureIndex = 0;
	}

}

void Model::Draw() {

	ID3D12GraphicsCommandList* commandList = modelCommon_->GetDirectXCommon()->GetCommandList();

	commandList->IASetVertexBuffers(0, 1, &vertexBufferView_);

	commandList->SetGraphicsRootConstantBufferView(0, materialResource_->GetGPUVirtualAddress());


	//if (modelData_.material.textureIndex != 0) {
	//	D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandle = TextureManager::GetInstance()->GetSrvHandleGPU(modelData_.material.textureIndex);
	//	commandList->SetGraphicsRootDescriptorTable(2, textureSrvHandle);
	//}
	uint32_t useTextureIndex = modelData_.material.textureIndex;
	if (useTextureIndex == 0) {
		useTextureIndex = 1; 
	}

	D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandle = TextureManager::GetInstance()->GetSrvHandleGPU(useTextureIndex);
	commandList->SetGraphicsRootDescriptorTable(2, textureSrvHandle);
	commandList->DrawInstanced(UINT(modelData_.vertices.size()), 1, 0, 0);

}

Model::MaterialData Model::LoadMaterialTemplate(const std::string& directoryPath, const std::string& filename)
{
	MaterialData materialData; 
	materialData.textureIndex = 0; 

	std::map<std::string, std::string> materials; 
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
		} else if (identifier == "map_Kd") {
			std::string textureFileName;
			s >> textureFileName;
			materialData.textureFilePath = directoryPath + "/" + textureFileName;
		}
	}
	return materialData;
}

Model::ModelData Model::LoadObjFile(const std::string& directoryPath, const std::string& filename)
{
	ModelData modelData; 
	std::vector<Vector4> positions;
	std::vector<Vector3> normals;
	std::vector<Vector2> texcoords;
	std::string line;

	std::string fullPath = filename;
	if (!directoryPath.empty()) {
		fullPath = directoryPath + "/" + filename;
	}
	std::ifstream file(fullPath);
	assert(file.is_open());
	std::string baseDirectory;
	size_t pos = fullPath.find_last_of('/');
	if (pos != std::string::npos) {
		baseDirectory = fullPath.substr(0, pos);
	} else {
		baseDirectory = ""; 
	}
	//std::ifstream file(directoryPath + "/" + filename);
	//assert(file.is_open());

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
		} else if (identifier == "vt") {
			Vector2 texcoord;
			s >> texcoord.x >> texcoord.y;
			texcoord.y = 1.0f - texcoord.y; // V-flip
			texcoords.push_back(texcoord);
		} else if (identifier == "vn") {
			Vector3 normal;
			s >> normal.x >> normal.y >> normal.z;
			normal.x *= -1.0f; // RH->LH
			normals.push_back(normal);
		} else if (identifier == "f") {
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

				triangle[faceVertex].position = positions[indices[0] - 1];

				if (indices[1] != 0) triangle[faceVertex].texcoord = texcoords[indices[1] - 1];
				else triangle[faceVertex].texcoord = { 0.0f, 0.0f };

				if (indices[2] != 0) triangle[faceVertex].normal = normals[indices[2] - 1];
				else triangle[faceVertex].normal = { 0.0f, 0.0f, 1.0f };

				// Sphere hack for smooth shading
				if (filename.find("sphere") != std::string::npos) {
					Vector3 p = { triangle[faceVertex].position.x, triangle[faceVertex].position.y, triangle[faceVertex].position.z };
					triangle[faceVertex].normal = Normalize(p);
				}
			}
			modelData.vertices.push_back(triangle[0]);
			modelData.vertices.push_back(triangle[2]);
			modelData.vertices.push_back(triangle[1]);
		} else if (identifier == "mtllib") {
			std::string materialFileName;
			s >> materialFileName;
			//modelData.material = LoadMaterialTemplate(directoryPath, materialFileName);
			modelData.material = LoadMaterialTemplate(baseDirectory, materialFileName);
		}
	}

	return modelData;
}
