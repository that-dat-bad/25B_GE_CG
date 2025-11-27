#pragma once
#include <string>
#include <cstdint>
#include "../base/Math/MyMath.h"
using namespace MyMath;

// パーティクル発生器
class ParticleEmitter {
public:
	// コンストラクタ（引数を追加）
	ParticleEmitter(const std::string& name, const Transform& transform, uint32_t count, float frequency,
		const Vector4& color = { 1.0f, 1.0f, 1.0f, 1.0f }, // デフォルト白
		const Vector3& velocity = { 0.0f, 1.0f, 0.0f },    // デフォルト上方向
		float velocityDiff = 0.0f);                      // デフォルト拡散なし

	// 更新処理
	void Update();

	// 発生させる
	void Emit();

private:
	std::string name_;      // 発生させるパーティクルグループ名
	Transform transform_;   // 発生位置などの情報
	uint32_t count_;        // 一度に発生させる数
	float frequency_;       // 発生頻度 (秒単位の間隔)
	float frequencyTime_;   // 経過時間計測用

	//パーティクルのパラメータ
	Vector4 color_;         // 色
	Vector3 velocity_;      // 速度
	float velocityDiff_;    // ランダム拡散値
};