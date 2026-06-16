#include "EnemyManager.h"

void EnemyManager::Initialize(const std::vector<EnemySpawnData>& spawnList) {
	enemies_.clear();
	enemies_.reserve(spawnList.size());

	for (const auto& data : spawnList) {
		auto enemy = std::make_unique<Enemy>();
		enemy->Initialize(data.position, data.modelPath, data.health);
		enemies_.push_back(std::move(enemy));
	}
}

void EnemyManager::Update() {
	for (auto& enemy : enemies_) {
		enemy->Update();
	}
}

void EnemyManager::Draw() {
	for (auto& enemy : enemies_) {
		enemy->Draw();
	}
}

int EnemyManager::GetAliveCount() const {
	int count = 0;
	for (const auto& enemy : enemies_) {
		if (enemy->IsAlive()) {
			++count;
		}
	}
	return count;
}

int EnemyManager::GetTotalCount() const {
	return static_cast<int>(enemies_.size());
}

int EnemyManager::GetDestroyedCount() const {
	return GetTotalCount() - GetAliveCount();
}

bool EnemyManager::IsAllDestroyed() const {
	return GetAliveCount() == 0 && GetTotalCount() > 0;
}

std::vector<Enemy*> EnemyManager::GetAliveEnemies() {
	std::vector<Enemy*> alive;
	for (auto& enemy : enemies_) {
		if (enemy->IsAlive()) {
			alive.push_back(enemy.get());
		}
	}
	return alive;
}
