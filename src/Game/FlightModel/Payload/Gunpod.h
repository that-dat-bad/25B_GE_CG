#pragma once
#include "IPayload.h"

struct GunPodData {
	float baseMass;     // ポッド本体の重さ
	float drag;         // 空気抵抗係数
	float ammoWeight;   // 弾丸1発の重さ
	int maxAmmo;        // 最大弾数
	float fireRate;     // 1秒間に何発撃てるか (例: 10.0f なら秒間10発)
};

class GunPod : public IPayload {
public:
	GunPod() : currentAmmo_(0), cooldownTimer_(0.0f), isAttached_(true) {}
	~GunPod() override = default;

	void Initialize(const GunPodData& data);

	// IPayloadのルール（仮想関数）を実装
	void Update(float dt) override;
	float GetAddedMass() const override;
	float GetAddedDrag() const override;
	bool IsAttached() const override { return isAttached_; }

	// トリガーを引いている間に呼ばれる関数
	void Fire();

private:
	float baseMass_;
	float drag_;
	float ammoWeight_;
	int currentAmmo_;

	float fireInterval_;  // 1発撃つための間隔（秒）
	float cooldownTimer_; // 次に撃てるようになるまでのタイマー

	bool isAttached_;
};