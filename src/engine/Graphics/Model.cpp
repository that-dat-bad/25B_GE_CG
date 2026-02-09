#include "Model.h"
#include "ModelCommon.h"
#include "DirectXCommon.h"
#include "TextureManager.h"
#include <cassert>
#include <map>
#include <fstream>
#include <sstream>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

void Model::Initialize(ModelCommon* modelCommon, const std::string& directorypath, const std::string& filename)
{

	modelCommon_ = modelCommon;
	DirectXCommon* dxCommon = modelCommon_->GetDirectXCommon();

	// 1. モデル読み込み
	modelData_ = LoadModelFile(directorypath, filename);

	// 2. 頂点データの初期化 (GPUリソース作成)

	// バッファリソース作成 (サイズは読み込んだ頂点数に合わせる)
	// 頂点がない場合のガード
	if (modelData_.vertices.empty()) {
		// ダミー頂点を作るか、アサートするか。
		// ここではアサートしておく
		// assert(false && "No vertices loaded");
		// あるいは何もしないでreturn (描画で落ちないようにDrawでガードが必要)
	}

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
	materialData_->enableLighting = true;
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
	uint32_t useTextureIndex = modelData_.material.textureIndex;
	if (useTextureIndex == 0) {
		useTextureIndex = 1; // 1番(uvChecker)を強制使用
	}

	D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandle = TextureManager::GetInstance()->GetSrvHandleGPU(useTextureIndex);
	commandList->SetGraphicsRootDescriptorTable(2, textureSrvHandle);
	// 描画コマンド発行
	commandList->DrawInstanced(UINT(modelData_.vertices.size()), 1, 0, 0);

}

Model::Node ReadNode(aiNode* node) {
	Model::Node result;
	aiMatrix4x4 aiLocalMatrix = node->mTransformation;
	aiLocalMatrix.Transpose(); // Row-major conversion
	
	// aiMatrix4x4 is struct with a1..d4. It also has operator[].
	// Assuming simple copy to MyMath::Matrix4x4 (float m[4][4])
	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 4; ++j) {
			result.localMatrix.m[i][j] = aiLocalMatrix[i][j];
		}
	}
	
	// X軸反転 (RH -> LH)
	result.localMatrix.m[3][0] *= -1.0f;
	result.localMatrix.m[0][1] *= -1.0f;
	result.localMatrix.m[0][2] *= -1.0f;
	result.localMatrix.m[1][0] *= -1.0f;
	result.localMatrix.m[2][0] *= -1.0f;

	result.name = node->mName.C_Str();
	result.children.resize(node->mNumChildren);
	for (uint32_t i = 0; i < node->mNumChildren; ++i) {
		result.children[i] = ReadNode(node->mChildren[i]);
	}
	return result;
}

Model::ModelData Model::LoadModelFile(const std::string& directoryPath, const std::string& filename)
{
	ModelData modelData;
	Assimp::Importer importer;
	std::string filePath = filename;
	if (!directoryPath.empty()) {
		filePath = directoryPath + "/" + filename;
	}

	const aiScene* scene = importer.ReadFile(filePath.c_str(), aiProcess_FlipWindingOrder | aiProcess_FlipUVs);
	assert(scene && scene->HasMeshes()); // 読み込み失敗時はassert

	// Mesh Analysis
	for (uint32_t meshIndex = 0; meshIndex < scene->mNumMeshes; ++meshIndex) {
		aiMesh* mesh = scene->mMeshes[meshIndex];
		assert(mesh->HasNormals());
		assert(mesh->HasTextureCoords(0));

		// Face Analysis
		for (uint32_t faceIndex = 0; faceIndex < mesh->mNumFaces; ++faceIndex) {
			aiFace& face = mesh->mFaces[faceIndex];
			assert(face.mNumIndices == 3);

			// Vertex Analysis
			for (uint32_t element = 0; element < face.mNumIndices; ++element) {
				uint32_t vertexIndex = face.mIndices[element];
				aiVector3D& position = mesh->mVertices[vertexIndex];
				aiVector3D& normal = mesh->mNormals[vertexIndex];
				aiVector3D& texcoord = mesh->mTextureCoords[0][vertexIndex];

				VertexData vertex;
				vertex.position = { position.x, position.y, position.z, 1.0f };
				vertex.normal = { normal.x, normal.y, normal.z };
				vertex.texcoord = { texcoord.x, texcoord.y };

				// RH -> LH
				vertex.position.x *= -1.0f;
				vertex.normal.x *= -1.0f;

				modelData.vertices.push_back(vertex);
			}
		}
	}

	// ディレクトリパスの取得 (テクスチャ読み込み用)
	std::string baseDirectory = "";
	size_t pos = filePath.find_last_of('/');
	if (pos != std::string::npos) {
		baseDirectory = filePath.substr(0, pos);
	}

	// Material Analysis
	for (uint32_t materialIndex = 0; materialIndex < scene->mNumMaterials; ++materialIndex) {
		aiMaterial* material = scene->mMaterials[materialIndex];
		if (material->GetTextureCount(aiTextureType_DIFFUSE) != 0) {
			aiString textureFilePath;
			material->GetTexture(aiTextureType_DIFFUSE, 0, &textureFilePath);
			
			// パス結合
			if (!baseDirectory.empty()) {
				modelData.material.textureFilePath = baseDirectory + "/" + textureFilePath.C_Str();
			} else {
				modelData.material.textureFilePath = textureFilePath.C_Str();
			}
		}
	}

	// Read Node Hierarchy
	modelData.rootNode = ReadNode(scene->mRootNode);

	return modelData;
}