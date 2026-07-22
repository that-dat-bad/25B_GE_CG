#include "AircraftVisualModel.h"
#include "../../engine/Graphics/Model/Object3dCommon.h"
#include "../../engine/Graphics/Model/ModelManager.h"
#include <numbers>

void AircraftVisualModel::Initialize(Object3dCommon* object3dCommon, Camera* camera) {
	object3dCommon_ = object3dCommon;
	camera_ = camera;
}

void AircraftVisualModel::SetModelForPart(DamagePart part, const std::string& modelFilePath) {
	Model* model = ModelManager::GetInstance()->FindModel(modelFilePath);
	if (!model) {
		ModelManager::GetInstance()->LoadModel(modelFilePath);
		model = ModelManager::GetInstance()->FindModel(modelFilePath);
	}
	SetModelForPart(part, model);
}

void AircraftVisualModel::SetModelForPart(DamagePart part, Model* model) {
	auto& node = partNodes_[part];
	if (!node.object) {
		node.object = std::make_unique<Object3d>();
		node.object->Initialize(object3dCommon_);
		if (camera_) {
			node.object->SetCamera(camera_);
		}
	}
	node.object->SetModel(model);
}

void AircraftVisualModel::SetPartLocalTransform(DamagePart part, const Vector3& scale, const Vector3& rotate, const Vector3& translate) {
	auto& node = partNodes_[part];
	node.localTransform.scale = scale;
	node.localTransform.rotate = rotate;
	node.localTransform.translate = translate;
}

void AircraftVisualModel::SetPartVisible(DamagePart part, bool visible) {
	auto it = partNodes_.find(part);
	if (it != partNodes_.end()) {
		it->second.isVisible = visible;
	}
}

bool AircraftVisualModel::IsPartVisible(DamagePart part) const {
	auto it = partNodes_.find(part);
	if (it != partNodes_.end()) {
		return it->second.isVisible;
	}
	return false;
}

void AircraftVisualModel::Update(const Matrix4x4& parentWorldMatrix, float deltaTime) {
	// プロペラ回転角の更新
	if (propellerRpm_ > 0.001f) {
		float revPerSec = propellerRpm_ / 60.0f;
		propellerAngle_ += revPerSec * 2.0f * std::numbers::pi_v<float> * deltaTime;
		if (propellerAngle_ > 2.0f * std::numbers::pi_v<float>) {
			propellerAngle_ -= 2.0f * std::numbers::pi_v<float>;
		}
	}

	for (auto& [part, node] : partNodes_) {
		if (!node.object || !node.isVisible) {
			continue;
		}

		// ローカル回転の計算（プロペラなどの自転も追加）
		Vector3 localRot = node.localTransform.rotate;
		if (part == DamagePart::Engine1) {
			localRot.z += propellerAngle_; // Roll軸周りに回転
		}

		Matrix4x4 localMatrix = MakeAffineMatrix(node.localTransform.scale, localRot, node.localTransform.translate);
		node.currentWorldMatrix = Multiply(localMatrix, parentWorldMatrix);

		node.object->UpdateWithWorldMatrix(node.currentWorldMatrix);
	}
}

void AircraftVisualModel::Draw() {
	for (auto& [part, node] : partNodes_) {
		if (node.object && node.isVisible) {
			node.object->Draw();
		}
	}
}

Matrix4x4 AircraftVisualModel::GetPartWorldMatrix(DamagePart part) const {
	auto it = partNodes_.find(part);
	if (it != partNodes_.end()) {
		return it->second.currentWorldMatrix;
	}
	return Identity4x4();
}

Vector3 AircraftVisualModel::GetPartWorldPosition(DamagePart part) const {
	Matrix4x4 world = GetPartWorldMatrix(part);
	return { world.m[3][0], world.m[3][1], world.m[3][2] };
}
