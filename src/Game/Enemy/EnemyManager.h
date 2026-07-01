#pragma once
#include <vector>
#include <memory>
#include <string>
#include "../../engine/base/Math/MyMath.h"
#include "Enemy.h"

/// @brief 敵の配置データ（コード内で直接定義する用）
struct EnemySpawnData {
	MyMath::Vector3 position;
	std::string modelPath;
	float health;
};

/// @brief 敵の生成・管理クラス
class EnemyManager {
public:
	EnemyManager() = default;
	~EnemyManager() = default;

	/// @brief 配置データから敵を一括生成
	void Initialize(
		const std::vector<EnemySpawnData>& spawnList,
		const AirframeData& airframeData,
		const EngineData& engineData,
		const GunPodData& gunpodData,
		FlightModel* playerFlightModel,
		BulletManager* bulletManager
	);

	/// @brief 全敵の更新
	void Update(float deltaTime);

	/// @brief 全敵の描画
	void Draw();

	// === ステータス照会 ===

	/// @brief 生存している敵の数
	int GetAliveCount() const;

	/// @brief 総敵数
	int GetTotalCount() const;

	/// @brief 撃破した敵の数
	int GetDestroyedCount() const;

	/// @brief 全敵が撃破されたか
	bool IsAllDestroyed() const;

	/// @brief 生存中の敵へのポインタリストを取得（衝突判定用）
	std::vector<Enemy*> GetAliveEnemies();

private:
	std::vector<std::unique_ptr<Enemy>> enemies_;
};
