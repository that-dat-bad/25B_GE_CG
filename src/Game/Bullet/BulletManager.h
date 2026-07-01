#pragma once
#include <vector>
#include <cstdint>
#include "Bullet.h"

class Camera;

/// @brief 弾丸のオブジェクトプール管理クラス
class BulletManager {
public:
	BulletManager() = default;
	~BulletManager() = default;

	/// @brief 初期化（オブジェクトプールの確保）
	/// @param maxBullets プールの最大数
	void Initialize(uint32_t maxBullets = 512);

	/// @brief 全弾丸の位置更新
	void Update(float dt);

	/// @brief 弾丸の描画（パーティクルインスタンシングで描画）
	void Draw(Camera* camera);

	/// @brief 弾丸を1発生成
	/// @param position 発射位置
	/// @param direction 発射方向（正規化済み）
	/// @param speed 弾速 (m/s)
	/// @param damage ダメージ
	/// @param isEnemyBullet 敵の弾かどうか
	void SpawnBullet(const MyMath::Vector3& position, const MyMath::Vector3& direction, float speed, float damage, bool isEnemyBullet = false);

	/// @brief アクティブな弾丸のリストへの参照を取得
	std::vector<Bullet>& GetBullets() { return bullets_; }

	/// @brief アクティブな弾丸数
	uint32_t GetActiveBulletCount() const;

private:
	void DrawTracerSegment(
		const MyMath::Vector3& p1,
		const MyMath::Vector3& p2,
		int segmentIndex,
		Camera* camera,
		uint32_t texIndex
	);

	std::vector<Bullet> bullets_;
	uint32_t maxBullets_ = 512;
};
