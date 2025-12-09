#include "FireworkParticle.h"
#include <cmath>
#include <algorithm>
#include <numbers> // pi_v

#include "Model.h"

using namespace MyMath;

FireworkParticle::~FireworkParticle() {
	for (auto p : particles_) delete p;
}

void FireworkParticle::Initialize(const Vector3 &position) {
  // ★ 実際のファイルパスに合わせて直してね
  // 例: ./Resources/firework/firework.obj とか
  std::string path = "./Resources/firework/firework.obj";

  // モデル読み込み
  Model::LoadFromOBJ(path);

  particles_.clear();
  particles_.reserve(kNumParticles);

  for (int i = 0; i < kNumParticles; ++i) {
    Object3d *obj = Object3d::Create();
    obj->SetModel(path); // ★ 同じパスを渡す
    obj->SetTranslate(position);
    obj->SetScale(Vector3{1.0f, 1.0f, 1.0f});
    obj->Update();
    particles_.push_back(obj);
  }

  color_ = {1.0f, 1.0f, 1.0f, 1.0f};
  counter_ = 0.0f;
  isFinished_ = false;
}

void FireworkParticle::Update() {
	if (isFinished_) return;

	counter_ += 1.0f / 60.0f;
	if (counter_ >= kDuration) {
		counter_ = kDuration;
		isFinished_ = true;
	}

	// 色更新
	color_.w = std::clamp(1.0f - (counter_ / kDuration), 0.0f, 1.0f);

	float angleUnit = (3.14159265f * 4.0f) / kNumParticles;

	for (int i = 0; i < kNumParticles; ++i) {
		// 速度計算
		float currentSpeed = (i >= kNumParticles / 2) ? 0.01f : speed_;
		Vector3 velocity = { currentSpeed, 0.0f, 0.0f };

		float angle = angleUnit * i;

		// 回転行列でベクトルを回す
		// MyMath::MakeRotateZMatrix が必要 (WorldTransformEx.cppにあったもの)
		// MyMath.h に追加実装するか、ここで計算
		Matrix4x4 matRot = MakeRotateZMatrix(angle);

		// Vector3 Transform(v, m) も MyMath に必要
		// ここでは簡易実装として展開
		Vector3 rotatedVel;
		rotatedVel.x = velocity.x * matRot.m[0][0] + velocity.y * matRot.m[1][0] + velocity.z * matRot.m[2][0];
		rotatedVel.y = velocity.x * matRot.m[0][1] + velocity.y * matRot.m[1][1] + velocity.z * matRot.m[2][1];
		rotatedVel.z = velocity.x * matRot.m[0][2] + velocity.y * matRot.m[1][2] + velocity.z * matRot.m[2][2];

		// 移動と回転
		Object3d* obj = particles_[i];
		obj->SetTranslate(Add(obj->GetTranslate(), rotatedVel));

		Vector3 rot = obj->GetRotate();
		rot.z = angle;
		obj->SetRotate(rot);

		obj->SetColor(color_);
		obj->Update();
	}
}

void FireworkParticle::Draw() {
	if (isFinished_) return;
	for (auto p : particles_) p->Draw();
}