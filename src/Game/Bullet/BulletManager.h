#pragma once
#include <vector>
#include <cstdint>
#include "Bullet.h"

class Camera;

/// <summary>
/// 弾丸のオブジェクトプール管理クラス
/// </summary>
class BulletManager {
public:
	BulletManager() = default;
	~BulletManager() = default;

	/// <summary>
	/// 初期化（オブジェクトプールの確保）
	/// </summary>
	/// <param name="maxBullets">プールの最大数</param>
	void Initialize(uint32_t maxBullets = 512);

	/// <summary>
	/// 全弾丸の位置更新
	/// </summary>
	void Update(float dt);

	/// <summary>
	/// 弾丸の描画（パーティクルインスタンシングで描画）
	/// </summary>
	void Draw(Camera* camera);

	/// <summary>
	/// 弾丸を1発生成
	/// </summary>
	/// <param name="position">発射位置</param>
	/// <param name="velocity">弾丸の初速度ベクトル (m/s)</param>
	/// <param name="damage">ダメージ</param>
	/// <param name="isEnemyBullet">敵の弾かどうか</param>
	void SpawnBullet(const MyMath::Vector3& position, const MyMath::Vector3& velocity, float damage, bool isEnemyBullet = false);

	/// <summary>
	/// アクティブな弾丸のリストへの参照を取得
	/// </summary>
	std::vector<Bullet>& GetBullets() { return bullets_; }

	/// <summary>
	/// アクティブな弾丸数
	/// </summary>
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
