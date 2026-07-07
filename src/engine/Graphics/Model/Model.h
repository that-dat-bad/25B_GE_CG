#pragma once
#include <d3d12.h>
#include <wrl/client.h>
#include <string>
#include <vector>
#include <map>
#include <span>
#include <optional>
#include <array>
#include "../base/Math/MyMath.h"

using namespace MyMath;
class ModelCommon;
class Camera;

class Model {
public:
	void Initialize(ModelCommon* modelCommon, const std::string& directorypath, const std::string& filename);
	void Draw();

	void DebugDrawSkeleton(const Matrix4x4& objectWorldMatrix, Camera* camera, const Vector4& color = { 1.0f, 1.0f, 1.0f, 1.0f });

	void SetShininess(float shininess) { materialData_->shininess = shininess; }
	float GetShininess() const { return materialData_->shininess; }

	void SetEnvironmentCoefficient(float coefficient) { materialData_->environmentCoefficient = coefficient; }
	float GetEnvironmentCoefficient() const { return materialData_->environmentCoefficient; }

	void SetSpecularIntensity(float intensity) { materialData_->specularIntensity = intensity; }
	float GetSpecularIntensity() const { return materialData_->specularIntensity; }

	struct VertexData {
		Vector4 position;
		Vector2 texcoord;
		Vector3 normal;
	};
	struct Material {
		Vector4 color;
		int32_t enableLighting;
		float shininess;
		float environmentCoefficient;
		float specularIntensity;
		Matrix4x4 uvTransform;
	};

	struct MaterialData {
		std::string textureFilePath;
		uint32_t textureIndex = 0;
		Vector4 baseColor = { 1.0f, 1.0f, 1.0f, 1.0f };
		float shininess = 50.0f;
		float environmentCoefficient = 0.0f;
		float specularIntensity = 1.0f;
	};

	struct QuaternionTransform {
		Vector3 scale;
		Quaternion rotate;
		Vector3 translate;
	};

	struct Node {
		QuaternionTransform transform;
		Matrix4x4 localMatrix = { 1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1 };
		std::string name;
		std::vector<Node> children;
	};

	struct VertexWeightData {
		float weight;
		uint32_t vertexIndex;
	};
	
	struct JointWeightData {
		Matrix4x4 inverseBindPoseMatrix;
		std::vector<VertexWeightData> vertexWeights;
	};

	struct ModelData {
		std::vector<VertexData> vertices;
		std::vector<uint32_t> indices;
		MaterialData material;
		Node rootNode;
		std::map<std::string, JointWeightData> skinClusterData;
	};

	template <typename tValue>
	struct Keyframe {
		float time;
		tValue value;
	};
	using KeyframeVector3 = Keyframe<Vector3>;
	using KeyframeQuaternion = Keyframe<Quaternion>;

	struct NodeAnimation {
		std::vector<KeyframeVector3> translate;
		std::vector<KeyframeQuaternion> rotate;
		std::vector<KeyframeVector3> scale;
	};

	struct Animation {
		float duration;
		std::map<std::string, NodeAnimation> nodeAnimations;
	};

	struct Joint {
		QuaternionTransform transform;
		Matrix4x4 localMatrix;
		Matrix4x4 skeletonSpaceMatrix;
		std::string name;
		std::vector<int32_t> children;
		int32_t index;
		std::optional<int32_t> parent;
	};

	struct Skeleton {
		int32_t root;
		std::map<std::string, int32_t> jointMap;
		std::vector<Joint> joints;
	};

	const static uint32_t kNumMaxInfluence = 4;
	struct VertexInfluence {
		std::array<float, kNumMaxInfluence> weights;
		std::array<int32_t, kNumMaxInfluence> jointIndices;
	};

	struct WellForGPU {
		Matrix4x4 skeletonSpaceMatrix;
		Matrix4x4 skeletonSpaceInverseTransposeMatrix;
	};

	struct SkinCluster {
		std::vector<Matrix4x4> inverseBindPoseMatrices;
		Microsoft::WRL::ComPtr<ID3D12Resource> influenceResource;
		D3D12_VERTEX_BUFFER_VIEW influenceBufferView;
		std::span<VertexInfluence> mappedInfluence;
		Microsoft::WRL::ComPtr<ID3D12Resource> paletteResource;
		std::span<WellForGPU> mappedPalette;
		uint32_t paletteSrvIndex = 0;
	};

	static ModelData LoadModelFile(const std::string& directoryPath, const std::string& filename);
	static Animation LoadAnimationFile(const std::string& directoryPath, const std::string& filename, const std::string& animationName);
	static std::vector<std::string> LoadAnimationNames(const std::string& directoryPath, const std::string& filename, const std::string& animationName = "");
	ModelData GetModelData() const { return modelData_; }

	void PlayAnimation(Animation* animation, float blendDuration = 0.0f);
	void Update(float deltaTime);
	
	bool IsSkinMesh() const { return isSkinMesh_; }

private:
	ModelCommon* modelCommon_ = nullptr;
	ModelData modelData_;
	bool isSkinMesh_ = false;

	Microsoft::WRL::ComPtr<ID3D12Resource> vertexBuffer_ = nullptr;
	VertexData* vertexData_ = nullptr;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_{};

	Microsoft::WRL::ComPtr<ID3D12Resource> indexBuffer_ = nullptr;
	D3D12_INDEX_BUFFER_VIEW indexBufferView_{};

	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource_ = nullptr;
	Material* materialData_ = nullptr;

	Skeleton skeleton_;
	SkinCluster skinCluster_;

	Animation* playingAnimation_ = nullptr;
	float animationTime_ = 0.0f;
	
	Animation* previousAnimation_ = nullptr;
	float previousAnimationTime_ = 0.0f;
	float blendDuration_ = 0.0f;
	float blendTimer_ = 0.0f;

	Skeleton CreateSkeleton(const Node& rootNode);
	int32_t CreateJoint(const Node& node, const std::optional<int32_t>& parent, std::vector<Joint>& joints);
	void UpdateSkeleton(Skeleton& skeleton);
	void ApplyAnimation(Skeleton& skeleton, const Animation& animation, float animationTime, float weight);
	SkinCluster CreateSkinCluster(const Skeleton& skeleton, const ModelData& modelData);
	void UpdateSkinCluster(SkinCluster& skinCluster, const Skeleton& skeleton);
	
	void DebugDrawNodeSkeleton(const Node& node, const Matrix4x4& parentMatrix, const Matrix4x4& objectWorldMatrix, Camera* camera, const Vector4& color);
};
