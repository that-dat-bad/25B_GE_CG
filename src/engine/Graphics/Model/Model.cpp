#include "Model.h"
#include "ModelCommon.h"
#include "../System/DirectXCommon.h"
#include "../System/TextureManager.h"
#include "../System/SrvManager.h"
#include "PrimitiveModel.h"
#include "../Camera/Camera.h"

#include <cassert>
#include <map>
#include <fstream>
#include <sstream>
#include <algorithm>
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

	if (modelData_.vertices.empty() || modelData_.indices.empty()) {
		return;
	}

	// 2. 頂点データの初期化 (GPUリソース作成)
	vertexBuffer_ = dxCommon->CreateBufferResource(sizeof(VertexData) * modelData_.vertices.size());
	vertexBufferView_.BufferLocation = vertexBuffer_->GetGPUVirtualAddress();
	vertexBufferView_.SizeInBytes = UINT(sizeof(VertexData) * modelData_.vertices.size());
	vertexBufferView_.StrideInBytes = sizeof(VertexData);

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

	// 6. テクスチャ読み込み
	if (!modelData_.material.textureFilePath.empty()) {
		TextureManager::GetInstance()->LoadTexture(modelData_.material.textureFilePath);
		modelData_.material.textureIndex = TextureManager::GetInstance()->GetTextureIndexByFilePath(modelData_.material.textureFilePath);
	} else {
		modelData_.material.textureIndex = 0;
	}

	// スキニング用データの初期化
	if (!modelData_.skinClusterData.empty()) {
		isSkinMesh_ = true;
		skeleton_ = CreateSkeleton(modelData_.rootNode);
		skinCluster_ = CreateSkinCluster(skeleton_, modelData_);
	} else {
		isSkinMesh_ = false;
	}
}

void Model::Draw() {
	if (modelData_.vertices.empty()) { return; }

	ID3D12GraphicsCommandList* commandList = modelCommon_->GetDirectXCommon()->GetCommandList();

	// インデックスバッファの設定
	commandList->IASetIndexBuffer(&indexBufferView_);

	// 頂点バッファの設定（スキニングの有無でバインド数を変える）
	if (isSkinMesh_) {
		D3D12_VERTEX_BUFFER_VIEW vbvs[2] = { vertexBufferView_, skinCluster_.influenceBufferView };
		commandList->IASetVertexBuffers(0, 2, vbvs);
		TextureManager::GetInstance()->GetSrvManager()->SetGraphicsRootDescriptorTable(9, skinCluster_.paletteSrvIndex);
	} else {
		commandList->IASetVertexBuffers(0, 1, &vertexBufferView_);
	}

	// マテリアルCBufferの設定 (RootParameter Index: 0)
	commandList->SetGraphicsRootConstantBufferView(0, materialResource_->GetGPUVirtualAddress());

	// テクスチャの設定 (RootParameter Index: 2)
	uint32_t useTextureIndex = modelData_.material.textureIndex;
	if (useTextureIndex == 0) {
		TextureManager::GetInstance()->LoadTexture("assets/textures/white1x1.png");
		useTextureIndex = TextureManager::GetInstance()->GetTextureIndexByFilePath("assets/textures/white1x1.png");
	}

	D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandle = TextureManager::GetInstance()->GetSrvHandleGPU(useTextureIndex);
	commandList->SetGraphicsRootDescriptorTable(2, textureSrvHandle);

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

	// ノードからSRTを抽出
	aiVector3D scale, translate;
	aiQuaternion rotate;
	node->mTransformation.Decompose(scale, rotate, translate);
	result.transform.scale = { scale.x, scale.y, scale.z };
	result.transform.rotate = { rotate.x, -rotate.y, -rotate.z, rotate.w }; // 左手系変換
	result.transform.translate = { -translate.x, translate.y, translate.z }; // 左手系変換

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
		aiMatrix4x4 globalTransform = node->mTransformation * parentTransform;

		for (uint32_t i = 0; i < node->mNumMeshes; ++i) {
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
			bool hasNormals = mesh->HasNormals();
			bool hasTexCoords = mesh->HasTextureCoords(0);
			uint32_t indexOffset = static_cast<uint32_t>(modelData.vertices.size());

			if (mesh->HasBones()) {
				for (uint32_t boneIndex = 0; boneIndex < mesh->mNumBones; ++boneIndex) {
					aiBone* ai_bone = mesh->mBones[boneIndex];
					std::string boneName = ai_bone->mName.C_Str();

					JointWeightData& jointWeightData = modelData.skinClusterData[boneName];
					aiMatrix4x4 m = ai_bone->mOffsetMatrix;
					
					// 右手系から左手系への変換を行いつつBindPoseを取得
					aiVector3D scale, translate;
					aiQuaternion rotate;
					m.Inverse().Decompose(scale, rotate, translate);
					Vector3 s = { scale.x, scale.y, scale.z };
					Quaternion r = { rotate.x, -rotate.y, -rotate.z, rotate.w };
					Vector3 t = { -translate.x, translate.y, translate.z };
					jointWeightData.inverseBindPoseMatrix = Inverse(MakeAffineMatrix(s, r, t));

					for (uint32_t weightIndex = 0; weightIndex < ai_bone->mNumWeights; ++weightIndex) {
						jointWeightData.vertexWeights.push_back({
							ai_bone->mWeights[weightIndex].mWeight,
							ai_bone->mWeights[weightIndex].mVertexId + indexOffset
						});
					}
				}
			}

			for (uint32_t vertexIndex = 0; vertexIndex < mesh->mNumVertices; ++vertexIndex) {
				aiVector3D position = mesh->mVertices[vertexIndex];

				VertexData vertex;
				vertex.position = { position.x, position.y, position.z, 1.0f };

				if (hasNormals) {
					aiVector3D normal = mesh->mNormals[vertexIndex];
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

				modelData.vertices.push_back(vertex);
			}

			for (uint32_t faceIndex = 0; faceIndex < mesh->mNumFaces; ++faceIndex) {
				aiFace& face = mesh->mFaces[faceIndex];
				if (face.mNumIndices != 3) { continue; }
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

	// ディレクトリパスの取得
	size_t pos = filePath.find_last_of('/');
	if (pos != std::string::npos) {
		baseDirectory = filePath.substr(0, pos);
	}

	if (scene->HasMaterials()) {
		aiMaterial* material = nullptr;
		if (scene->HasMeshes() && scene->mNumMeshes > 0) {
			material = scene->mMaterials[scene->mMeshes[0]->mMaterialIndex];
		} else {
			material = scene->mMaterials[0];
		}

		if (material->GetTextureCount(aiTextureType_DIFFUSE) != 0) {
			aiString textureFilePath;
			material->GetTexture(aiTextureType_DIFFUSE, 0, &textureFilePath);

			if (textureFilePath.C_Str()[0] == '*') {
				uint32_t textureIndex = static_cast<uint32_t>(std::stoi(textureFilePath.C_Str() + 1));
				if (textureIndex < scene->mNumTextures) {
					aiTexture* embeddedTexture = scene->mTextures[textureIndex];
					std::string embeddedTexName = absolutePath + "_tex" + std::to_string(textureIndex);

					if (embeddedTexture->mHeight == 0) {
						TextureManager::GetInstance()->LoadTextureFromMemory(
							embeddedTexName,
							embeddedTexture->pcData,
							embeddedTexture->mWidth
						);
					}
					modelData.material.textureFilePath = embeddedTexName;
				}
			} else {
				std::string texPath;
				if (!baseDirectory.empty()) {
					texPath = baseDirectory + "/" + textureFilePath.C_Str();
				} else {
					texPath = textureFilePath.C_Str();
				}

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

		aiColor4D color;
		if (AI_SUCCESS == material->Get(AI_MATKEY_BASE_COLOR, color)) {
			modelData.material.baseColor = { color.r, color.g, color.b, color.a };
		} else if (AI_SUCCESS == material->Get(AI_MATKEY_COLOR_DIFFUSE, color)) {
			modelData.material.baseColor = { color.r, color.g, color.b, color.a };
		}

		float roughness = 1.0f;
		if (AI_SUCCESS == material->Get(AI_MATKEY_ROUGHNESS_FACTOR, roughness)) {
			if (roughness >= 0.99f) {
				modelData.material.shininess = 1.0f;
			} else {
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
		std::string errorOut = "Assimp Error: No animation found\n" + std::string(importer.GetErrorString()) + "\nFailed to load: " + absolutePath;
		MessageBoxA(nullptr, errorOut.c_str(), "Animation Load Error", MB_OK | MB_ICONERROR);
		return animation;
	}

	aiAnimation* aiAnimation = nullptr;
	if (!animationName.empty()) {
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
			keyframe.value = { -float(key.mValue.x), float(key.mValue.y), float(key.mValue.z) }; // 左手系変換
			nodeAnimation.translate.push_back(keyframe);
		}

		for (uint32_t keyframeIndex = 0; keyframeIndex < nodeAnim->mNumRotationKeys; ++keyframeIndex) {
			aiQuatKey& key = nodeAnim->mRotationKeys[keyframeIndex];
			KeyframeQuaternion keyframe;
			keyframe.time = float(key.mTime) / ticksPerSecond;
			keyframe.value = { float(key.mValue.x), -float(key.mValue.y), -float(key.mValue.z), float(key.mValue.w) }; // 左手系変換
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

std::vector<std::string> Model::LoadAnimationNames(const std::string& directoryPath, const std::string& filename, const std::string& animationName) {
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

void Model::PlayAnimation(Animation* animation, float fadeDuration) {
	previousAnimation_ = playingAnimation_;
	previousAnimationTime_ = animationTime_;
	playingAnimation_ = animation;
	animationTime_ = 0.0f;
	blendDuration_ = fadeDuration;
	blendTimer_ = 0.0f;
}

void Model::Update(float deltaTime) {
	if (!playingAnimation_) { return; }

	animationTime_ += deltaTime;
	animationTime_ = std::fmod(animationTime_, playingAnimation_->duration);

	if (blendDuration_ > 0.0f) {
		blendTimer_ += deltaTime;
		if (blendTimer_ >= blendDuration_) {
			blendDuration_ = 0.0f;
		} else {
			if (previousAnimation_) {
				previousAnimationTime_ += deltaTime;
				previousAnimationTime_ = std::fmod(previousAnimationTime_, previousAnimation_->duration);
			}
		}
	}

	if (isSkinMesh_) {
		if (blendDuration_ > 0.0f && previousAnimation_) {
			float weight = blendTimer_ / blendDuration_;
			ApplyAnimation(skeleton_, *previousAnimation_, previousAnimationTime_, 1.0f - weight);
			ApplyAnimation(skeleton_, *playingAnimation_, animationTime_, weight);
		} else {
			ApplyAnimation(skeleton_, *playingAnimation_, animationTime_, 1.0f);
		}
		UpdateSkeleton(skeleton_);
		UpdateSkinCluster(skinCluster_, skeleton_);
	}
}

Model::Skeleton Model::CreateSkeleton(const Node& rootNode) {
	Skeleton skeleton;
	skeleton.root = CreateJoint(rootNode, {}, skeleton.joints);

	for (const Joint& joint : skeleton.joints) {
		skeleton.jointMap.emplace(joint.name, joint.index);
	}
	return skeleton;
}

int32_t Model::CreateJoint(const Node& node, const std::optional<int32_t>& parent, std::vector<Joint>& joints) {
	Joint joint;
	joint.name = node.name;
	joint.localMatrix = node.localMatrix;
	joint.skeletonSpaceMatrix = Identity4x4();
	joint.transform = node.transform;
	joint.index = static_cast<int32_t>(joints.size());
	joint.parent = parent;
	joints.push_back(joint);

	for (const Node& child : node.children) {
		int32_t childIndex = CreateJoint(child, joint.index, joints);
		joints[joint.index].children.push_back(childIndex);
	}
	return joint.index;
}

void Model::UpdateSkeleton(Skeleton& skeleton) {
	for (Joint& joint : skeleton.joints) {
		joint.localMatrix = MakeAffineMatrix(joint.transform.scale, joint.transform.rotate, joint.transform.translate);
		if (joint.parent) {
			joint.skeletonSpaceMatrix = joint.localMatrix * skeleton.joints[*joint.parent].skeletonSpaceMatrix;
		} else {
			joint.skeletonSpaceMatrix = joint.localMatrix;
		}
	}
}

void Model::ApplyAnimation(Skeleton& skeleton, const Animation& animation, float time, float weight) {
	for (Joint& joint : skeleton.joints) {
		if (animation.nodeAnimations.contains(joint.name)) {
			const NodeAnimation& anim = animation.nodeAnimations.at(joint.name);
			Vector3 scale = CalculateScale(anim.scale, time);
			Quaternion rotate = CalculateRotate(anim.rotate, time);
			Vector3 translate = CalculateTranslate(anim.translate, time);

			if (weight >= 1.0f) {
				joint.transform.scale = scale;
				joint.transform.rotate = rotate;
				joint.transform.translate = translate;
			} else if (weight > 0.0f) {
				joint.transform.scale = Lerp(joint.transform.scale, scale, weight);
				joint.transform.rotate = Slerp(joint.transform.rotate, rotate, weight);
				joint.transform.translate = Lerp(joint.transform.translate, translate, weight);
			}
		}
	}
}

Model::SkinCluster Model::CreateSkinCluster(const Skeleton& skeleton, const ModelData& modelData) {
	SkinCluster skinCluster;
	DirectXCommon* dxCommon = modelCommon_->GetDirectXCommon();

	skinCluster.paletteResource = dxCommon->CreateBufferResource(sizeof(WellForGPU) * skeleton.joints.size());
	WellForGPU* paletteData = nullptr;
	skinCluster.paletteResource->Map(0, nullptr, reinterpret_cast<void**>(&paletteData));
	skinCluster.mappedPalette = std::span<WellForGPU>(paletteData, skeleton.joints.size());

	skinCluster.paletteSrvIndex = TextureManager::GetInstance()->GetSrvManager()->Allocate();
	TextureManager::GetInstance()->GetSrvManager()->CreateSRVforStructuredBuffer(
		skinCluster.paletteSrvIndex,
		skinCluster.paletteResource.Get(),
		UINT(skeleton.joints.size()),
		sizeof(WellForGPU)
	);

	skinCluster.influenceResource = dxCommon->CreateBufferResource(sizeof(VertexInfluence) * modelData.vertices.size());
	VertexInfluence* influenceData = nullptr;
	skinCluster.influenceResource->Map(0, nullptr, reinterpret_cast<void**>(&influenceData));
	skinCluster.mappedInfluence = std::span<VertexInfluence>(influenceData, modelData.vertices.size());

	std::memset(influenceData, 0, sizeof(VertexInfluence) * modelData.vertices.size());

	skinCluster.influenceBufferView.BufferLocation = skinCluster.influenceResource->GetGPUVirtualAddress();
	skinCluster.influenceBufferView.SizeInBytes = UINT(sizeof(VertexInfluence) * modelData.vertices.size());
	skinCluster.influenceBufferView.StrideInBytes = sizeof(VertexInfluence);

	skinCluster.inverseBindPoseMatrices.resize(skeleton.joints.size());
	for(size_t i = 0; i < skeleton.joints.size(); i++) {
		skinCluster.inverseBindPoseMatrices[i] = Identity4x4();
	}

	for (const auto& jointWeight : modelData.skinClusterData) {
		auto it = skeleton.jointMap.find(jointWeight.first);
		if (it == skeleton.jointMap.end()) { continue; }
		uint32_t jointIndex = it->second;
		skinCluster.inverseBindPoseMatrices[jointIndex] = jointWeight.second.inverseBindPoseMatrix;

		for (const auto& vertexWeight : jointWeight.second.vertexWeights) {
			auto& currentInfluence = skinCluster.mappedInfluence[vertexWeight.vertexIndex];
			for (uint32_t index = 0; index < kNumMaxInfluence; ++index) {
				if (currentInfluence.weights[index] == 0.0f) {
					currentInfluence.weights[index] = vertexWeight.weight;
					currentInfluence.jointIndices[index] = jointIndex;
					break;
				}
			}
		}
	}
	return skinCluster;
}

void Model::UpdateSkinCluster(SkinCluster& skinCluster, const Skeleton& skeleton) {
	for (size_t jointIndex = 0; jointIndex < skeleton.joints.size(); ++jointIndex) {
		skinCluster.mappedPalette[jointIndex].skeletonSpaceMatrix =
			skinCluster.inverseBindPoseMatrices[jointIndex] * skeleton.joints[jointIndex].skeletonSpaceMatrix;
		
		Matrix4x4 m = skinCluster.mappedPalette[jointIndex].skeletonSpaceMatrix;
		Matrix4x4 inv = Inverse(m);
		Matrix4x4 transpose = {
			inv.m[0][0], inv.m[1][0], inv.m[2][0], inv.m[3][0],
			inv.m[0][1], inv.m[1][1], inv.m[2][1], inv.m[3][1],
			inv.m[0][2], inv.m[1][2], inv.m[2][2], inv.m[3][2],
			inv.m[0][3], inv.m[1][3], inv.m[2][3], inv.m[3][3]
		};
		skinCluster.mappedPalette[jointIndex].skeletonSpaceInverseTransposeMatrix = transpose;
	}
}

void Model::DebugDrawSkeleton(const Matrix4x4& objectWorldMatrix, Camera* camera, const Vector4& color) {
	if (modelData_.rootNode.children.empty()) { return; }
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

	Matrix4x4 parentFinal = parentMatrix * objectWorldMatrix;
	Vector3 parentPos = { parentFinal.m[3][0], parentFinal.m[3][1], parentFinal.m[3][2] };

	Matrix4x4 myFinal = globalMatrix * objectWorldMatrix;
	Vector3 myPos = { myFinal.m[3][0], myFinal.m[3][1], myFinal.m[3][2] };

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
