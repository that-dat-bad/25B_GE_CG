#pragma once
#include <string>
#include <cstdint>
#include "../base/Math/MyMath.h"
using namespace MyMath;

#include "ParticleManager.h"

// パーティクル発生器
class ParticleEmitter {
public:
	// コンストラクタで設定を受け取る
	ParticleEmitter(const std::string& name, const Transform& transform, uint32_t count, float frequency);

	// 更新処理
	void Update();

	// 発生させる
	void Emit();

	/// <summary>
	/// パラメータの設定
	/// </summary>
	/// <param name="params">パーティクルパラメータ</param>
	void SetParticleParameters(const ParticleParameters& params) { params_ = params; }

private:
	std::string name_;      // 発生させるパーティクルグループ名
	Transform transform_;   // 発生位置などの情報
	uint32_t count_;        // 一度に発生させる数
	float frequency_;       // 発生頻度 (秒単位の間隔)
	float frequencyTime_;   // 経過時間計測用

	ParticleParameters params_; // 発生させるパーティクルのパラメータ
};