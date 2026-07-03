#pragma once
#include <memory>
#include <string>
#include <list>
#include "../base/Math/MyMath.h"
#include "../System/BlendMode.h"
#include "../Model/PrimitiveModel.h"
#include "ParticleManager.h"
#include "../Camera/CameraManager.h"

using namespace MyMath;

class IEffect {
public:
	virtual ~IEffect() = default;
	virtual void Update() = 0;
	virtual void Draw(Camera* camera) = 0;
	bool IsDead() const { return isDead_; }

protected:
	bool isDead_ = false;
	Vector3 position_ = { 0,0,0 };
	float lifeTime_ = 1.0f;
	float currentTime_ = 0.0f;
};

// ============================================
// リングエフェクト
// ============================================
class RingEffect : public IEffect {
public:
	RingEffect(const Vector3& pos, const Vector4& color, float lifeTime, float maxRadius);
	void Update() override;
	void Draw(Camera* camera) override;
private:
	Vector4 color_;
	uint32_t textureIndex_;
	float currentRadius_ = 0.0f;
	float maxRadius_ = 5.0f;
	Vector3 scale_{ 0,0,0 };
};

// ============================================
// シリンダー柱エフェクト
// ============================================
class CylinderEffect : public IEffect {
public:
	CylinderEffect(const Vector3& pos, const Vector4& color, float lifeTime, float maxScaleY, float maxScaleX);
	void Update() override;
	void Draw(Camera* camera) override;
private:
	Vector4 color_;
	uint32_t textureIndex_;
	Vector3 scale_{ 0,0,0 };
	float maxScaleY_ = 10.0f;
	float maxScaleX_ = 2.0f;
};

// ============================================
// 回転するPlane(斬撃)エフェクト
// ============================================
class HitRotPlaneEffect : public IEffect {
public:
	HitRotPlaneEffect(const Vector3& pos, const Vector4& color, float lifeTime, float size);
	void Update() override;
	void Draw(Camera* camera) override;
private:
	Vector4 color_;
	uint32_t textureIndex_;
	Vector3 rotationEulers_{ 0,0,0 };
	Vector3 rotationSpeed_{ 0,0,0 };
	Vector3 scale_{ 1,1,1 };
	float targetSize_ = 2.0f;
};

// ============================================
// ビルボードパーティクルエフェクト (ラッパー)
// ============================================
class BillboardParticleEffect : public IEffect {
public:
	BillboardParticleEffect(const Vector3& pos, const std::string& groupName, uint32_t count);
	void Update() override;
	void Draw(Camera* camera) override;
};

// ============================================
// 爆発パーティクルエフェクト（火花+炎+煙の複合演出）
// ============================================
class ExplosionParticleEffect : public IEffect {
public:
	ExplosionParticleEffect(const Vector3& pos);
	void Update() override;
	void Draw(Camera* camera) override;

private:
	float flashScale_ = 1.0f;
	float flashAlpha_ = 1.0f;
	
	float shockwaveScale_ = 1.0f;
	float shockwaveAlpha_ = 1.0f;

	struct Debris {
		Vector3 pos;
		Vector3 vel;
	};
	std::vector<Debris> debris_;
	
	float lightIntensity_ = 0.0f;
};

// ============================================
// マズルフラッシュエフェクト（発砲時の火花+閃光）
// ============================================
class MuzzleFlashEffect : public IEffect {
public:
	MuzzleFlashEffect(const Vector3& pos, const Vector3& direction);
	void Update() override;
	void Draw(Camera* camera) override;
private:
	Vector3 direction_;
};

/// <summary>
/// エフェクト管理クラス (各種パーティクル・ジオメトリ演出制御)
/// </summary>
class EffectManager {
public:
	/// <summary>
	/// シングルトンインスタンスの取得
	/// </summary>
	/// <returns>インスタンスポインタ</returns>
	static EffectManager* GetInstance();

	/// <summary>
	/// デフォルトコンストラクタ (std::make_unique対応のためpublic)
	/// </summary>
	EffectManager() = default;

	void Initialize();
	void Update();
	void Draw(Camera* camera);
	void Finalize();

	/// <summary>
	/// エフェクトの追加 (所有権を委譲する)
	/// </summary>
	/// <param name="effect">追加するエフェクトのunique_ptr</param>
	void AddEffect(std::unique_ptr<IEffect> effect);

	// 複数のエフェクトを組み合わせた関数
	void EmitHitEffect(const Vector3& position);
	void EmitHitPlaneEffect(const Vector3& position);

	// 敵破壊時の大きな爆発エフェクト
	void EmitDestroyEffect(const Vector3& position);

	// 発砲時のマズルフラッシュエフェクト
	void EmitMuzzleFlash(const Vector3& position, const Vector3& direction);

	~EffectManager() = default;
private:
	EffectManager(const EffectManager&) = delete;
	EffectManager& operator=(const EffectManager&) = delete;

	static std::unique_ptr<EffectManager> instance_;

	std::list<std::unique_ptr<IEffect>> effects_;
};
