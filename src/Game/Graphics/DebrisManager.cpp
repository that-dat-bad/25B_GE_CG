#include "DebrisManager.h"
#include "../../engine/Graphics/Model/Object3dCommon.h"
#include "../../engine/Graphics/Particle/EffectManager.h"
#include <random>

std::unique_ptr<DebrisManager> DebrisManager::instance_ = nullptr;

DebrisManager* DebrisManager::GetInstance() {
	if (!instance_) {
		instance_ = std::unique_ptr<DebrisManager>(new DebrisManager());
	}
	return instance_.get();
}

void DebrisObject::Update(float deltaTime) {
	lifetime -= deltaTime;

	// 重力と空気抵抗
	velocity.y -= gravity * deltaTime;
	velocity = Multiply(1.0f - drag * deltaTime, velocity);

	// 移動
	position = Add(position, Multiply(deltaTime, velocity));
	rotation = Add(rotation, Multiply(deltaTime, angularVelocity));

	// ワールド行列の更新
	Matrix4x4 world = MakeAffineMatrix(scale, rotation, position);
	if (object) {
		object->UpdateWithWorldMatrix(world);
	}

	// 定期的に航跡煙エフェクトを発生
	emitSmokeTimer += deltaTime;
	if (emitSmokeTimer >= 0.1f) {
		emitSmokeTimer = 0.0f;
		if (EffectManager::GetInstance()) {
			EffectManager::GetInstance()->EmitHitPlaneEffect(position);
		}
	}
}

void DebrisObject::Draw() {
	if (object) {
		object->Draw();
	}
}

void DebrisManager::Initialize(Object3dCommon* object3dCommon, Camera* camera) {
	object3dCommon_ = object3dCommon;
	camera_ = camera;
	debrisList_.clear();
}

void DebrisManager::Update(float deltaTime) {
	for (auto it = debrisList_.begin(); it != debrisList_.end();) {
		(*it)->Update(deltaTime);
		if ((*it)->IsDead()) {
			it = debrisList_.erase(it);
		} else {
			++it;
		}
	}
}

void DebrisManager::Draw() {
	for (auto& debris : debrisList_) {
		debris->Draw();
	}
}

void DebrisManager::Clear() {
	debrisList_.clear();
}

void DebrisManager::SpawnDebris(Model* model, const Matrix4x4& initialWorldMatrix, const Vector3& baseVelocity, const Vector3& ejectionForce) {
	if (!model || !object3dCommon_) return;

	auto debris = std::make_unique<DebrisObject>();
	debris->object = std::make_unique<Object3d>();
	debris->object->Initialize(object3dCommon_);
	if (camera_) {
		debris->object->SetCamera(camera_);
	}
	debris->object->SetModel(model);

	// 行列から位置抽出
	debris->position = { initialWorldMatrix.m[3][0], initialWorldMatrix.m[3][1], initialWorldMatrix.m[3][2] };
	debris->rotation = { 0.0f, 0.0f, 0.0f }; // 初期状態

	// ランダムな自転角速度
	static std::random_device rd;
	static std::mt19937 gen(rd());
	std::uniform_real_distribution<float> rotDist(-5.0f, 5.0f);
	std::uniform_real_distribution<float> randForce(-3.0f, 3.0f);

	Vector3 randomVec = { randForce(gen), randForce(gen), randForce(gen) };
	debris->velocity = Add(Add(baseVelocity, ejectionForce), randomVec);
	debris->angularVelocity = { rotDist(gen), rotDist(gen), rotDist(gen) };
	debris->lifetime = 6.0f;

	debrisList_.push_back(std::move(debris));
}
