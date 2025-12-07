#pragma once
#include <random>

class Rand {
private:
	// 乱数生成エンジン
	std::random_device seedGenerator_;

	// メルセンヌ・ツイスターエンジン(64bit版)
	std::mt19937_64 randomEngine;

	// 指定範囲の乱数生成器
	std::uniform_real_distribution<float> rotationDistribution;

public:
	void Initialize();
	void RandomInitialize();
	float GetRandom();
};