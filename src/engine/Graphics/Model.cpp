#include "Model.h"
#include "ModelCommon.h"
#include "DirectXCommon.h"
#include "TextureManager.h"
#include "ModelManager.h" 
#include "WorldTransform.h"
#include "Camera.h"
#include "ObjectColor.h" // 追加: ObjectColorを使用
#include "TDEngine.h"    // GetObject3dCommonを使用

#include <cassert>
#include <fstream>
#include <sstream>
#include <filesystem> 

using namespace TDEngine;
using namespace MyMath;

// 静的関数: モデル生成 (ModelManager経由)
Model* Model::CreateFromOBJ(const std::string& modelName, bool smoothing) {
	// smoothing引数は現状未使用ですが、互換性のため残します
	(void)smoothing;

	// マネージャ経由でロード
	ModelManager::GetInstance()->LoadModel(modelName);

	// ロードされたモデルのポインタを返す
	return ModelManager::GetInstance()->FindModel(modelName);
}

// 静的関数: ロードのみ
void Model::LoadFromOBJ(const std::string& modelName) {
	ModelManager::GetInstance()->LoadModel(modelName);
}

// 初期化
void Model::Initialize(ModelCommon* modelCommon, const std::string& directorypath, const std::string& filename) {
	modelCommon_ = modelCommon;

	// OBJファイルの読み込み
	modelData_ = LoadObjFile(directorypath, filename);

	// 頂点バッファ生成
	vertexBuffer_ = modelCommon_->GetDirectXCommon()->CreateBufferResource(sizeof(VertexData) * modelData_.vertices.size());

	// 頂点バッファビュー作成
	vertexBufferView_.BufferLocation = vertexBuffer_->GetGPUVirtualAddress();
	vertexBufferView_.SizeInBytes = UINT(sizeof(VertexData) * modelData_.vertices.size());
	vertexBufferView_.StrideInBytes = sizeof(VertexData);

	// 頂点データ書き込み
	VertexData* vertexData = nullptr;
	vertexBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
	std::memcpy(vertexData, modelData_.vertices.data(), sizeof(VertexData) * modelData_.vertices.size());
	vertexBuffer_->Unmap(0, nullptr);

	// マテリアル用定数バッファ生成
	materialResource_ = modelCommon_->GetDirectXCommon()->CreateBufferResource(sizeof(Material));

	// マテリアルデータ初期化・書き込み
	materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&materialData_));
	materialData_->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	materialData_->enableLighting = 1; // 1:有効 0:無効
	materialData_->shininess = 1.0f;
	materialData_->uvTransform = Identity4x4();
}

// 描画 (引数なし版)
void Model::Draw() {
	// 基本的には引数あり版を使う想定ですが、安全のため空実装かデフォルト動作を記述
	// ここでは何もしないか、あるいはデフォルトのカメラ等があればそれを使って描画
}

// ★修正: ObjectColor対応のDraw
void Model::Draw(const WorldTransform& worldTransform, const Camera& camera, const ObjectColor* objectColor) {

	// 1. WVP行列の計算と転送 (WorldTransform側のバッファ更新)
	// ※ WorldTransformクラスの仕様に合わせて、ここで計算して書き込む
	if (worldTransform.constMap) {
		Matrix4x4 worldViewProjection = Multiply(worldTransform.matWorld_, camera.GetViewProjectionMatrix());
		worldTransform.constMap->WVP = worldViewProjection;
		worldTransform.constMap->matWorld = worldTransform.matWorld_;
	}

	// 2. 共通設定 (パイプラインステートなど)
	// Object3dCommonの設定を流用
	TDEngine::GetObject3dCommon()->SetupCommonState();

	ID3D12GraphicsCommandList* commandList = modelCommon_->GetDirectXCommon()->GetCommandList();

	// 3. 頂点バッファセット
	commandList->IASetVertexBuffers(0, 1, &vertexBufferView_);

	// 4. マテリアル(色)用の定数バッファセット (RootParameter[0])
	if (objectColor) {
		// ObjectColorが指定された場合、そのGPUアドレスを使用
		commandList->SetGraphicsRootConstantBufferView(0, objectColor->GetGPUVirtualAddress());
	}
	else {
		// 指定なしの場合、モデルが持つデフォルトのマテリアルバッファを使用
		commandList->SetGraphicsRootConstantBufferView(0, materialResource_->GetGPUVirtualAddress());
	}

	// 5. Transform用の定数バッファセット (RootParameter[1])
	commandList->SetGraphicsRootConstantBufferView(1, worldTransform.constBuff_->GetGPUVirtualAddress());

	// 6. テクスチャセット (RootParameter[2])
	if (modelData_.material.textureIndex != 0) {
		D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandle = TextureManager::GetInstance()->GetSrvHandleGPU(modelData_.material.textureIndex);
		commandList->SetGraphicsRootDescriptorTable(2, textureSrvHandle);
	}

	// 7. 描画コマンド発行
	commandList->DrawInstanced(UINT(modelData_.vertices.size()), 1, 0, 0);
}

// 透明度設定 (デフォルトマテリアル用)
void Model::SetAlpha(float alpha) {
	if (materialData_) {
		materialData_->color.w = alpha;
	}
}

// OBJファイル読み込み
Model::ModelData Model::LoadObjFile(const std::string& directoryPath, const std::string& filename) {
	ModelData modelData;
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
			// 座標系の変換 (RH -> LH) 必要に応じてXを反転など
			position.x *= -1.0f;
			positions.push_back(position);
		}
		else if (identifier == "vt") {
			Vector2 texcoord;
			s >> texcoord.x >> texcoord.y;
			texcoord.y = 1.0f - texcoord.y; // Y反転
			texcoords.push_back(texcoord);
		}
		else if (identifier == "vn") {
			Vector3 normal;
			s >> normal.x >> normal.y >> normal.z;
			normal.x *= -1.0f; // 座標系合わせ
			normals.push_back(normal);
		}
		else if (identifier == "f") {
			VertexData triangle[3];
			for (int32_t faceVertex = 0; faceVertex < 3; ++faceVertex) {
				std::string vertexDefinition;
				s >> vertexDefinition;

				std::istringstream v(vertexDefinition);
				std::string index;

				// 位置
				std::getline(v, index, '/');
				triangle[faceVertex].position = positions[std::stoi(index) - 1];

				// UV
				std::getline(v, index, '/');
				if (!index.empty()) {
					triangle[faceVertex].texcoord = texcoords[std::stoi(index) - 1];
				}

				// 法線
				std::getline(v, index, '/');
				if (!index.empty()) {
					triangle[faceVertex].normal = normals[std::stoi(index) - 1];
				}
			}
			// 頂点法線の反転等の処理が必要ならここで行う
			// 三角形を追加
			modelData.vertices.push_back(triangle[2]);
			modelData.vertices.push_back(triangle[1]);
			modelData.vertices.push_back(triangle[0]);
		}
		else if (identifier == "mtllib") {
			std::string materialFilename;
			s >> materialFilename;
			modelData.material = LoadMaterialTemplate(directoryPath, materialFilename);
		}
	}
	return modelData;
}

// マテリアル読み込み
Model::MaterialData Model::LoadMaterialTemplate(const std::string& directoryPath, const std::string& filename) {
	MaterialData materialData;
	std::string line;
	std::ifstream file(directoryPath + "/" + filename);

	// マテリアルファイルがない、または開けない場合はデフォルトテクスチャを使用
	if (!file.is_open()) {
		// 白画像などをロードして割り当てると良いが、ここでは0(なし)か適当な値を返す
		materialData.textureIndex = 0;
		return materialData;
	}

	while (std::getline(file, line)) {
		std::string identifier;
		std::istringstream s(line);
		s >> identifier;

		if (identifier == "map_Kd") {
			std::string textureFilename;
			s >> textureFilename;
			materialData.textureFilePath = directoryPath + "/" + textureFilename;
			materialData.textureIndex = TextureManager::Load(materialData.textureFilePath);
		}
	}
	return materialData;
}