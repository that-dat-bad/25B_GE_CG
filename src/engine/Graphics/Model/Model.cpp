#include "Model.h"
#include "ModelCommon.h"
#include "../System/DirectXCommon.h"
#include "../System/TextureManager.h"
#include "PrimitiveModel.h"
#include "../Camera/Camera.h"

#include <cassert>
#include <map>
#include <fstream>
#include <sstream>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <Windows.h>
#include <filesystem>
#include <functional>



//--補助関数群--


// 時間に合わせて「移動」を計算する
Vector3 CalculateTranslate(const std::vector<Model::KeyframeVector3>& keys, float time) {
	if (keys.empty()) { return { 0,0,0 }; }
	if (keys.size() == 1 || time <= keys.front().time) { return keys.front().value; }
	if (time >= keys.back().time) { return keys.back().value; }

	for (size_t i = 0; i < keys.size() - 1; ++i) {
		if (time >= keys[i].time && time <= keys[i + 1].time) {
			float t = (time - keys[i].time) / (keys[i + 1].time - keys[i].time);
			return Lerp(keys[i].value, keys[i + 1].value, t);
		}
	}
	return keys.back().value;
}

// 時間に合わせて「回転」を計算する
Quaternion CalculateRotate(const std::vector<Model::KeyframeQuaternion>& keys, float time) {
	if (keys.empty()) { return { 0,0,0,1 }; }
	if (keys.size() == 1 || time <= keys.front().time) { return keys.front().value; }
	if (time >= keys.back().time) { return keys.back().value; }

	for (size_t i = 0; i < keys.size() - 1; ++i) {
		if (time >= keys[i].time && time <= keys[i + 1].time) {
			float t = (time - keys[i].time) / (keys[i + 1].time - keys[i].time);
			return Slerp(keys[i].value, keys[i + 1].value, t);
		}
	}
	return keys.back().value;
}

// 時間に合わせて「拡縮」を計算する
Vector3 CalculateScale(const std::vector<Model::KeyframeVector3>& keys, float time) {
	if (keys.empty()) { return { 1,1,1 }; }
	if (keys.size() == 1 || time <= keys.front().time) { return keys.front().value; }
	if (time >= keys.back().time) { return keys.back().value; }

	for (size_t i = 0; i < keys.size() - 1; ++i) {
		if (time >= keys[i].time && time <= keys[i + 1].time) {
			float t = (time - keys[i].time) / (keys[i + 1].time - keys[i].time);
			return Lerp(keys[i].value, keys[i + 1].value, t);
		}
	}
	return keys.back().value;
}




void Model::Initialize(ModelCommon* modelCommon, const std::string& directorypath, const std::string& filename) {

	modelCommon_ = modelCommon;
	DirectXCommon* dxCommon = modelCommon_->GetDirectXCommon();

	// 1. モデル読み込み
	modelData_ = LoadModelFile(directorypath, filename);

	// 2. 頂点データの初期化 (GPUリソース作成)

	// バッファリソース作成 (サイズは読み込んだ頂点数に合わせる)
	// 頂点がない場合のガード
	if (modelData_.vertices.empty() || modelData_.indices.empty()) {
		return;
	}

	vertexBuffer_ = dxCommon->CreateBufferResource(sizeof(VertexData) * modelData_.vertices.size());

	vertexBufferView_.BufferLocation = vertexBuffer_->GetGPUVirtualAddress();
	vertexBufferView_.SizeInBytes = UINT(sizeof(VertexData) * modelData_.vertices.size());
	vertexBufferView_.StrideInBytes = sizeof(VertexData);

	// データの書き込み
	vertexBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData_));
	std::memcpy(vertexData_, modelData_.vertices.data(), sizeof(VertexData) * modelData_.vertices.size());
	vertexBuffer_->Unmap(0, nullptr);

	// インデックスバッファの初期化
	indexBuffer_ = dxCommon->CreateBufferResource(sizeof(uint32_t) * modelData_.indices.size());

	indexBufferView_.BufferLocation = indexBuffer_->GetGPUVirtualAddress();
	indexBufferView_.SizeInBytes = UINT(sizeof(uint32_t) * modelData_.indices.size());
	indexBufferView_.Format = DXGI_FORMAT_R32_UINT;

	uint32_t* indexMap = nullptr;
	indexBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&indexMap));
	std::memcpy(indexMap, modelData_.indices.data(), sizeof(uint32_t) * modelData_.indices.size());
	indexBuffer_->Unmap(0, nullptr);

	// 3. マテリアルの初期化

	materialResource_ = dxCommon->CreateBufferResource(sizeof(Material));
	materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&materialData_));

	materialData_->color = modelData_.material.baseColor;
	materialData_->enableLighting = true;
	materialData_->shininess = modelData_.material.shininess;
	materialData_->environmentCoefficient = modelData_.material.environmentCoefficient;
	materialData_->specularIntensity = modelData_.material.specularIntensity;
	materialData_->uvTransform = Identity4x4();
	materialData_->environmentCoefficient = 0.0f;

	// 6. テクスチャ読み込み

	if (!modelData_.material.textureFilePath.empty()) {
		TextureManager::GetInstance()->LoadTexture(modelData_.material.textureFilePath);
		modelData_.material.textureIndex = TextureManager::GetInstance()->GetTextureIndexByFilePath(modelData_.material.textureFilePath);
	} else {
		modelData_.material.textureIndex = 0;
	}

	//7.ボーン用のバッファ作成
	boneResource_ = dxCommon->CreateBufferResource(sizeof(BoneMatrix));
	boneResource_->Map(0, nullptr, reinterpret_cast<void**>(&boneData_));
	for (int i = 0; i < 100; i++) {
		boneData_->matrices[i] = Identity4x4();
	}

}

void Model::Draw() {
	if (modelData_.vertices.empty()) { return; }

	// コマンドリストの取得
	ID3D12GraphicsCommandList* commandList = modelCommon_->GetDirectXCommon()->GetCommandList();

	// 頂点バッファの設定
	commandList->IASetVertexBuffers(0, 1, &vertexBufferView_);
	// インデックスバッファの設定
	commandList->IASetIndexBuffer(&indexBufferView_);

	// マテリアルCBufferの設定 (RootParameter Index: 0)
	commandList->SetGraphicsRootConstantBufferView(0, materialResource_->GetGPUVirtualAddress());


	// テクスチャ (DescriptorTable) の設定 (RootParameter Index: 2)
	uint32_t useTextureIndex = modelData_.material.textureIndex;
	if (useTextureIndex == 0) {
		// テクスチャなしマテリアル: 白1x1テクスチャを使い、マテリアルカラーがそのまま出るようにする
		TextureManager::GetInstance()->LoadTexture("assets/textures/white1x1.png");
		useTextureIndex = TextureManager::GetInstance()->GetTextureIndexByFilePath("assets/textures/white1x1.png");
	}

	D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandle = TextureManager::GetInstance()->GetSrvHandleGPU(useTextureIndex);
	commandList->SetGraphicsRootDescriptorTable(2, textureSrvHandle);

	commandList->SetGraphicsRootConstantBufferView(7, boneResource_->GetGPUVirtualAddress());
	// 描画コマンド発行
	commandList->DrawIndexedInstanced(UINT(modelData_.indices.size()), 1, 0, 0, 0);

}

Model::Node ReadNode(aiNode* node) {
	Model::Node result;
	aiMatrix4x4 aiLocalMatrix = node->mTransformation;
	aiLocalMatrix.Transpose();

	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 4; ++j) {
			result.localMatrix.m[i][j] = aiLocalMatrix[i][j];
		}
	}



	result.name = node->mName.C_Str();
	result.children.resize(node->mNumChildren);
	for (uint32_t i = 0; i < node->mNumChildren; ++i) {
		result.children[i] = ReadNode(node->mChildren[i]);
	}
	return result;
}

Model::ModelData Model::LoadModelFile(const std::string& directoryPath, const std::string& filename) {
	ModelData modelData;
	Assimp::Importer importer;
	std::string filePath = filename;
	//テクスチャのためにディレクトリを抽出
	std::string baseDirectory = std::filesystem::path(filePath).parent_path().string();
	if (!directoryPath.empty()) {
		filePath = directoryPath + "/" + filename;
	}

	std::string absolutePath = std::filesystem::absolute(filePath).string();

	const aiScene* scene = importer.ReadFile(absolutePath.c_str(),
		aiProcess_FlipWindingOrder | aiProcess_FlipUVs | aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_JoinIdenticalVertices);
	if (!scene || !scene->HasMeshes()) {
		return modelData;
	}

	std::function<void(aiNode*, const aiMatrix4x4&)> processNode = [&](aiNode* node, const aiMatrix4x4& parentTransform) {
		// 行列の掛け合わせの順序は (ローカル * 親)
		aiMatrix4x4 globalTransform = node->mTransformation * parentTransform;

		for (uint32_t i = 0; i < node->mNumMeshes; ++i) {
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
			bool hasNormals = mesh->HasNormals();
			bool hasTexCoords = mesh->HasTextureCoords(0);

			std::vector<std::vector<std::pair<uint32_t, float>>> vertexWeightMap(mesh->mNumVertices);

			if (mesh->HasBones()) {
				for (uint32_t boneIndex = 0; boneIndex < mesh->mNumBones; ++boneIndex) {
					aiBone* ai_bone = mesh->mBones[boneIndex];
					std::string boneName = ai_bone->mName.C_Str();

					// 1. ボーンを登録してインデックスを決める
					uint32_t myBoneIndex = 0;
					if (!modelData.boneIndexMap.contains(boneName)) {
						myBoneIndex = static_cast<uint32_t>(modelData.bones.size());
						modelData.boneIndexMap[boneName] = myBoneIndex;

						ModelData::BoneData boneData;
						boneData.name = boneName;
						aiMatrix4x4 m = ai_bone->mOffsetMatrix;
						boneData.inverseBindPoseMatrix = {
							m.a1, m.b1, m.c1, m.d1,
							m.a2, m.b2, m.c2, m.d2,
							m.a3, m.b3, m.c3, m.d3,
							m.a4, m.b4, m.c4, m.d4
						};
						modelData.bones.push_back(boneData);
					} else {
						myBoneIndex = modelData.boneIndexMap[boneName];
					}

					// 2. このボーンが影響を与える頂点を、事前整理リストに追加していく
					for (uint32_t weightIndex = 0; weightIndex < ai_bone->mNumWeights; ++weightIndex) {
						const aiVertexWeight& weight = ai_bone->mWeights[weightIndex];
						vertexWeightMap[weight.mVertexId].push_back({ myBoneIndex, weight.mWeight });
					}
				}
			}
			// =================================================================

			uint32_t indexOffset = static_cast<uint32_t>(modelData.vertices.size());

			// 頂点の抽出
			for (uint32_t vertexIndex = 0; vertexIndex < mesh->mNumVertices; ++vertexIndex) {
				aiVector3D position = mesh->mVertices[vertexIndex];


				VertexData vertex;
				vertex.position = { position.x, position.y, position.z, 1.0f };

				if (hasNormals) {
					aiVector3D normal = mesh->mNormals[vertexIndex];
					aiMatrix3x3 normalMatrix = aiMatrix3x3(globalTransform);
					normal.Normalize();
					vertex.normal = { normal.x, normal.y, normal.z };
				} else {
					vertex.normal = { 0.0f, 1.0f, 0.0f };
				}

				if (hasTexCoords) {
					aiVector3D& texcoord = mesh->mTextureCoords[0][vertexIndex];
					vertex.texcoord = { texcoord.x, texcoord.y };
				} else {
					vertex.texcoord = { 0.0f, 0.0f };
				}

				vertex.position.x *= -1.0f;
				vertex.normal.x *= -1.0f;

				// アニメーション関連初期化 (ゼロクリア)
				vertex.weight = { 1.0f, 0.0f, 0.0f, 0.0f };
				vertex.indices[0] = 0;
				vertex.indices[1] = 0;
				vertex.indices[2] = 0;
				vertex.indices[3] = 0;

				auto& weights = vertexWeightMap[vertexIndex];
				for (size_t w = 0; w < weights.size(); ++w) {
					if (w >= 4) { break; }

					if (w == 0) { vertex.weight.x = weights[w].second; vertex.indices[0] = weights[w].first; } else if (w == 1) { vertex.weight.y = weights[w].second; vertex.indices[1] = weights[w].first; } else if (w == 2) { vertex.weight.z = weights[w].second; vertex.indices[2] = weights[w].first; } else if (w == 3) { vertex.weight.w = weights[w].second; vertex.indices[3] = weights[w].first; }
				}
				// =================================================================

				modelData.vertices.push_back(vertex);
			}

			// 面（三角形）の展開 (インデックス抽出)
			for (uint32_t faceIndex = 0; faceIndex < mesh->mNumFaces; ++faceIndex) {
				aiFace& face = mesh->mFaces[faceIndex];
				if (face.mNumIndices != 3) { continue; } // 三角形以外はスキップ

				for (uint32_t element = 0; element < face.mNumIndices; ++element) {
					modelData.indices.push_back(face.mIndices[element] + indexOffset);
				}
			}
		}

		for (uint32_t i = 0; i < node->mNumChildren; ++i) {
			processNode(node->mChildren[i], globalTransform);
		}
		};

	aiMatrix4x4 identity;
	processNode(scene->mRootNode, identity);

	if (!modelData.vertices.empty()) {
		float minX = modelData.vertices[0].position.x;
		float maxX = minX;
		float minY = modelData.vertices[0].position.y;
		float maxY = minY;
		float minZ = modelData.vertices[0].position.z;
		float maxZ = minZ;
		for (const auto& v : modelData.vertices) {
			if (v.position.x < minX) { minX = v.position.x; }
			if (v.position.x > maxX) { maxX = v.position.x; }
			if (v.position.y < minY) { minY = v.position.y; }
			if (v.position.y > maxY) { maxY = v.position.y; }
			if (v.position.z < minZ) { minZ = v.position.z; }
			if (v.position.z > maxZ) { maxZ = v.position.z; }
		}
		char buf[256];
		sprintf_s(buf, "[Model.cpp] Bound Box for %s: X[%.3f, %.3f], Y[%.3f, %.3f], Z[%.3f, %.3f], Verts:%zu\n",
			absolutePath.c_str(), minX, maxX, minY, maxY, minZ, maxZ, modelData.vertices.size());
		OutputDebugStringA(buf);
	}

	// ディレクトリパスの取得 (テクスチャ読み込み用)
	size_t pos = filePath.find_last_of('/');
	if (pos != std::string::npos) {
		baseDirectory = filePath.substr(0, pos);
	}

	if (scene->HasMaterials()) {
		// 最初のメッシュが使用しているマテリアルを取得する
		aiMaterial* material = nullptr;
		if (scene->HasMeshes() && scene->mNumMeshes > 0) {
			material = scene->mMaterials[scene->mMeshes[0]->mMaterialIndex];
		} else {
			material = scene->mMaterials[0];
		}

		//テクスチャパスの取得（元の処理そのまま）
// ① テクスチャパスの取得と埋め込みテクスチャの処理
		if (material->GetTextureCount(aiTextureType_DIFFUSE) != 0) {
			aiString textureFilePath;
			material->GetTexture(aiTextureType_DIFFUSE, 0, &textureFilePath);

			if (textureFilePath.C_Str()[0] == '*') {
				// "*0" のような文字列から数値インデックスを取得
				uint32_t textureIndex = static_cast<uint32_t>(std::stoi(textureFilePath.C_Str() + 1));

				if (textureIndex < scene->mNumTextures) {
					aiTexture* embeddedTexture = scene->mTextures[textureIndex];

					// テクスチャに一意の識別子(名前)をつける (例: "Duck.glb_tex0")
					std::string embeddedTexName = absolutePath + "_tex" + std::to_string(textureIndex);

					if (embeddedTexture->mHeight == 0) {
						// 1. ここでメモリから直接テクスチャを作ってVRAMに送る！
						TextureManager::GetInstance()->LoadTextureFromMemory(
							embeddedTexName,
							embeddedTexture->pcData,
							embeddedTexture->mWidth
						);
					}

					// 2. エンジン側には「この識別子のテクスチャを使う」と登録しておく
					modelData.material.textureFilePath = embeddedTexName;
				}
			}
			// 外部ファイル読み込み (今までの処理)
			else {
				// パス結合
				std::string texPath;
				if (!baseDirectory.empty()) {
					texPath = baseDirectory + "/" + textureFilePath.C_Str();
				} else {
					texPath = textureFilePath.C_Str();
				}

				// ファイルが存在しなければ assets/textures/ にフォールバック
				if (!std::filesystem::exists(texPath)) {
					std::string texName = std::filesystem::path(textureFilePath.C_Str()).filename().string();
					std::string fallback = "assets/textures/" + texName;
					if (std::filesystem::exists(fallback)) {
						texPath = fallback;
					}
				}

				modelData.material.textureFilePath = texPath;
			}
		}

		//ベースカラーの取得
		aiColor4D color;
		if (AI_SUCCESS == material->Get(AI_MATKEY_BASE_COLOR, color)) {
			modelData.material.baseColor = { color.r, color.g, color.b, color.a };
		} else if (AI_SUCCESS == material->Get(AI_MATKEY_COLOR_DIFFUSE, color)) {
			modelData.material.baseColor = { color.r, color.g, color.b, color.a };
		}

		// 粗さ(Roughness)の取得と Shininess への変換
		float roughness = 1.0f; // デフォルトはザラザラ
		if (AI_SUCCESS == material->Get(AI_MATKEY_ROUGHNESS_FACTOR, roughness)) {
			if (roughness >= 0.99f) {
				modelData.material.shininess = 1.0f; // ツヤなし
			} else {
				// 粗さが低い(ツルツル)ほどShininessを高くする変換
				modelData.material.shininess = 2.0f / powf(roughness, 4.0f) - 2.0f;
				if (modelData.material.shininess < 1.0f) { modelData.material.shininess = 1.0f; }
				if (modelData.material.shininess > 256.0f) { modelData.material.shininess = 256.0f; }
			}
		} else {
			float shininess = 0.0f;
			if (AI_SUCCESS == material->Get(AI_MATKEY_SHININESS, shininess)) {
				if (shininess < 1.0f) { shininess = 1.0f; }
				if (shininess > 256.0f) { shininess = 256.0f; }
				modelData.material.shininess = shininess;
			}
		}
	}


	modelData.rootNode = ReadNode(scene->mRootNode);

	return modelData;
}

Model::Animation Model::LoadAnimationFile(const std::string& directoryPath, const std::string& filename, const std::string& animationName) {
	Animation animation;
	Assimp::Importer importer;
	std::string filePath = filename;
	if (!directoryPath.empty()) {
		filePath = directoryPath + "/" + filename;
	}

	std::string absolutePath = std::filesystem::absolute(filePath).string();

	const aiScene* scene = importer.ReadFile(absolutePath.c_str(), 0);
	if (!scene || scene->mNumAnimations == 0) {
		std::string errorOut = "Assimp Error: No animation found\n";
		errorOut += importer.GetErrorString();
		errorOut += "\nFailed to load: " + absolutePath;
		MessageBoxA(nullptr, errorOut.c_str(), "Animation Load Error", MB_OK | MB_ICONERROR);
		return animation;
	}

	aiAnimation* aiAnimation = nullptr;

	if (!animationName.empty()) {
		// 名前が指定されている場合はループして探す
		for (uint32_t i = 0; i < scene->mNumAnimations; ++i) {
			if (std::string(scene->mAnimations[i]->mName.C_Str()) == animationName) {
				aiAnimation = scene->mAnimations[i];
				break;
			}
		}
	}
	if (!aiAnimation) {
		aiAnimation = scene->mAnimations[0];
	}

	float ticksPerSecond = 1.0f;
	if (aiAnimation->mTicksPerSecond != 0.0) {
		ticksPerSecond = float(aiAnimation->mTicksPerSecond);
	}
	animation.duration = float(aiAnimation->mDuration) / ticksPerSecond;

	for (uint32_t channelIndex = 0; channelIndex < aiAnimation->mNumChannels; ++channelIndex) {
		aiNodeAnim* nodeAnim = aiAnimation->mChannels[channelIndex];
		NodeAnimation& nodeAnimation = animation.nodeAnimations[nodeAnim->mNodeName.C_Str()];

		for (uint32_t keyframeIndex = 0; keyframeIndex < nodeAnim->mNumPositionKeys; ++keyframeIndex) {
			aiVectorKey& key = nodeAnim->mPositionKeys[keyframeIndex];
			KeyframeVector3 keyframe;
			keyframe.time = float(key.mTime) / ticksPerSecond;
			keyframe.value = { float(key.mValue.x), float(key.mValue.y), float(key.mValue.z) };
			nodeAnimation.translate.push_back(keyframe);
		}

		for (uint32_t keyframeIndex = 0; keyframeIndex < nodeAnim->mNumRotationKeys; ++keyframeIndex) {
			aiQuatKey& key = nodeAnim->mRotationKeys[keyframeIndex];
			KeyframeQuaternion keyframe;
			keyframe.time = float(key.mTime) / ticksPerSecond;
			keyframe.value = { float(key.mValue.x), float(key.mValue.y), float(key.mValue.z), float(key.mValue.w) };
			nodeAnimation.rotate.push_back(keyframe);
		}

		for (uint32_t keyframeIndex = 0; keyframeIndex < nodeAnim->mNumScalingKeys; ++keyframeIndex) {
			aiVectorKey& key = nodeAnim->mScalingKeys[keyframeIndex];
			KeyframeVector3 keyframe;
			keyframe.time = float(key.mTime) / ticksPerSecond;
			keyframe.value = { float(key.mValue.x), float(key.mValue.y), float(key.mValue.z) };
			nodeAnimation.scale.push_back(keyframe);
		}
	}

	return animation;
}

std::vector<std::string> Model::LoadAnimationNames(const std::string& directoryPath, const std::string& filename,const std::string& animationName) {
	std::vector<std::string> names;
	Assimp::Importer importer;
	std::string filePath = filename;
	if (!directoryPath.empty()) {
		filePath = directoryPath + "/" + filename;
	}

	std::string absolutePath = std::filesystem::absolute(filePath).string();

	const aiScene* scene = importer.ReadFile(absolutePath.c_str(), 0);
	if (!scene || scene->mNumAnimations == 0) {
		return names;
	}

	for (uint32_t i = 0; i < scene->mNumAnimations; ++i) {
		names.push_back(scene->mAnimations[i]->mName.C_Str());
	}

	return names;
}

void Model::PlayAnimation(Animation* animation) {
	playingAnimation_ = animation;
	animationTime_ = 0.0f;
}

void Model::Update(float deltaTime) {
	if (!playingAnimation_) { return; }

	// 1. 時間を進める
	animationTime_ += deltaTime;
	// ループ再生：再生時間がアニメーションの長さを超えたら最初に戻る
	animationTime_ = std::fmod(animationTime_, playingAnimation_->duration);

	// 2. 全てのボーンの行列を計算する
	// ここで階層構造(Node)をたどって、今のポーズの行列を作成
	UpdateNodeAnimation(modelData_.rootNode, Identity4x4());
}

void Model::UpdateNodeAnimation(const Node& node, const Matrix4x4& parentMatrix) {
	Matrix4x4 localMatrix = node.localMatrix;

	// もしアニメーションデータが存在するなら、時間に合わせて計算する
	if (playingAnimation_ && playingAnimation_->nodeAnimations.contains(node.name)) {
		const NodeAnimation& anim = playingAnimation_->nodeAnimations.at(node.name);

		Vector3 scale = CalculateScale(anim.scale, animationTime_);
		Quaternion rotate = CalculateRotate(anim.rotate, animationTime_);
		Vector3 translate = CalculateTranslate(anim.translate, animationTime_);

		localMatrix = MakeAffineMatrix(scale, rotate, translate);
	}

	// ワールド行列の計算
	Matrix4x4 globalMatrix = localMatrix * parentMatrix;

	// ボーンの行列を書き込む
	if (modelData_.boneIndexMap.contains(node.name)) {
		uint32_t index = modelData_.boneIndexMap[node.name];
		boneData_->matrices[index] = modelData_.bones[index].inverseBindPoseMatrix * globalMatrix;
	}

	// 子供たちへ再帰
	for (const auto& child : node.children) {
		UpdateNodeAnimation(child, globalMatrix);
	}
}

void Model::DebugDrawSkeleton(const Matrix4x4& objectWorldMatrix, Camera* camera, const Vector4& color) {
	if (modelData_.rootNode.children.empty()) { return; }
	// ルートノードから再帰的に描画
	DebugDrawNodeSkeleton(modelData_.rootNode, Identity4x4(), objectWorldMatrix, camera, color);
}

void Model::DebugDrawNodeSkeleton(const Node& node, const Matrix4x4& parentMatrix, const Matrix4x4& objectWorldMatrix, Camera* camera, const Vector4& color) {
	Matrix4x4 localMatrix = node.localMatrix;

	if (playingAnimation_ && playingAnimation_->nodeAnimations.contains(node.name)) {
		const NodeAnimation& anim = playingAnimation_->nodeAnimations.at(node.name);

		Vector3 scale = CalculateScale(anim.scale, animationTime_);
		Quaternion rotate = CalculateRotate(anim.rotate, animationTime_);
		Vector3 translate = CalculateTranslate(anim.translate, animationTime_);

		localMatrix = MakeAffineMatrix(scale, rotate, translate);
	}

	Matrix4x4 globalMatrix = localMatrix * parentMatrix;

	// 親のワールド座標
	Matrix4x4 parentFinal = parentMatrix * objectWorldMatrix;
	Vector3 parentPos = { parentFinal.m[3][0], parentFinal.m[3][1], parentFinal.m[3][2] };

	// 自分のワールド座標
	Matrix4x4 myFinal = globalMatrix * objectWorldMatrix;
	Vector3 myPos = { myFinal.m[3][0], myFinal.m[3][1], myFinal.m[3][2] };

	// 線を描画（親と自分の位置が同じなら描画しない。ルートの初回など）
	float distSq = (myPos.x - parentPos.x) * (myPos.x - parentPos.x) +
				   (myPos.y - parentPos.y) * (myPos.y - parentPos.y) +
				   (myPos.z - parentPos.z) * (myPos.z - parentPos.z);
	
	if (distSq > 0.0001f) {
		PrimitiveModel::GetInstance()->DrawLine3D(parentPos, myPos, color, camera);
	}

	for (const auto& child : node.children) {
		DebugDrawNodeSkeleton(child, globalMatrix, objectWorldMatrix, camera, color);
	}
}
