#include "Rand.h"
#include <random>

void Rand::Initialize() {
	// メルセンヌ・ツイスターエンジンの初期化
	randomEngine.seed(seedGenerator_());
}

void Rand::RandomInitialize() { rotationDistribution = std::uniform_real_distribution<float>(1.0f, 8.0f); }

float Rand::GetRandom() { return rotationDistribution(randomEngine); }