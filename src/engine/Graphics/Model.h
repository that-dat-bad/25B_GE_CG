#pragma once
#include <d3d12.h>
#include <wrl/client.h>
#include <string>
#include <vector>
#include <map>
#include "../base/Math/MyMath.h"

using namespace MyMath;
class ModelCommon;
class Camera;

class Model {
public:
	void Initialize(ModelCommon* modelCommon, const std::string& directorypath, const std::string& filename);
	void Draw();

	// デバッグ用: スケルトンの描画
	void DebugDrawSkeleton(const Matrix4x4& objectWorldMatrix, Camera* camera, const Vector4& color = { 1.0f, 1.0f, 1.0f, 1.0f });

	void SetShininess(float shininess) { materialData_->shininess = shininess; }
	float GetShininess() const { return materialData_->shininess; }

	void SetEnvironmentCoefficient(float coefficient) { materialData_->environmentCoefficient = coefficient; }
	float GetEnvironmentCoefficient() const { return materialData_->environmentCoefficient; }

	struct VertexData {
		Vector4 position;
		Vector2 texcoord;
		Vector3 normal;
		Vector4 weight;
		int32_t indices[4];
	};
	struct Material {
		Vector4 color;
		int32_t enableLighting;
		float shininess;
		float environmentCoefficient;
		float padding;
		Matrix4x4 uvTransform; // UV変換行列
	};

	struct BoneMatrix {
		Matrix4x4 matrices[100];
	};
	struct MaterialData {
		std::string textureFilePath;
		uint32_t textureIndex = 0;
		Vector4 baseColor = { 1.0f, 1.0f, 1.0f, 1.0f }; //デフォルトは白
		float shininess = 50.0f;
		float environmentCoefficient = 0.0f; //環境マップの反射度合い
	};
	struct Node {
		Matrix4x4 localMatrix = { 1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1 };
		std::string name;
		std::vector<Node> children;
	};
	struct ModelData {
		std::vector<VertexData> vertices;
		std::vector<uint32_t> indices;
		MaterialData material;
		Node rootNode;
		struct BoneData {
			std::string name;
			Matrix4x4 inverseBindPoseMatrix;//初期姿勢の逆行列
		};
		std::vector<BoneData> bones;//ボーンの配列
		std::map < std::string, uint32_t> boneIndexMap;//ボーン名からインデックスへのマップ
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

	static ModelData LoadModelFile(const std::string& directoryPath, const std::string& filename);
	static Animation LoadAnimationFile(const std::string& directoryPath, const std::string& filename, const std::string& animationName);
	static std::vector<std::string> LoadAnimationNames(const std::string& directoryPath, const std::string& filename, const std::string& animationName = "");
	ModelData GetModelData() const { return modelData_; }

	// アニメーションを再生スタートする関数
	void PlayAnimation(Animation* animation);

	// 毎フレーム呼んでアニメーションを進める関数
	void Update(float deltaTime);
private:

	ModelCommon* modelCommon_ = nullptr;

	// モデルデータ
	ModelData modelData_;

	//--頂点データ--//
	// バッファリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexBuffer_ = nullptr;
	// バッファリソース内のデータを指すポインタ
	VertexData* vertexData_ = nullptr;
	// バッファビュー
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_{};

	//--インデックスデータ--//
	Microsoft::WRL::ComPtr<ID3D12Resource> indexBuffer_ = nullptr;
	D3D12_INDEX_BUFFER_VIEW indexBufferView_{};

	//--マテリアル--//
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource_ = nullptr;
	Material* materialData_ = nullptr;


	Microsoft::WRL::ComPtr<ID3D12Resource> boneResource_;
	BoneMatrix* boneData_ = nullptr;

	// 現在再生中のアニメーションデータ
	Animation* playingAnimation_ = nullptr;
	// 現在の再生時間（秒）
	float animationTime_ = 0.0f;
	void UpdateNodeAnimation(const Node& node, const Matrix4x4& parentMatrix);
	void DebugDrawNodeSkeleton(const Node& node, const Matrix4x4& parentMatrix, const Matrix4x4& objectWorldMatrix, Camera* camera, const Vector4& color);
};