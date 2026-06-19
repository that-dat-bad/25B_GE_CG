#include "BulletManager.h"
#include "../../engine/Graphics/PrimitiveModel.h"
#include "../../engine/Graphics/TextureManager.h"
#include "../../engine/Graphics/Camera.h"
#include "../../engine/Graphics/BlendMode.h"

using namespace MyMath;

void BulletManager::Initialize(uint32_t maxBullets) {
	maxBullets_ = maxBullets;
	bullets_.resize(maxBullets_);
	// 全弾丸を非活性状態で初期化（Bulletのデフォルトコンストラクタで isAlive_ = false）
}

void BulletManager::Update(float dt) {
	for (auto& bullet : bullets_) {
		bullet.Update(dt);
	}
}

void BulletManager::Draw(Camera* camera) {
	if (!camera) return;

	// 丸いパーティクル用テクスチャ（マズルフラッシュと同じもの）
	uint32_t texIndex = TextureManager::GetInstance()->GetTextureIndexByFilePath("assets/textures/circle2.png");

	// アクティブな弾丸をビルボードのパーティクル球として描画
	for (const auto& bullet : bullets_) {
		if (!bullet.IsAlive()) continue;

		Vector3 pos = bullet.GetPosition();
		Vector3 vel = bullet.GetVelocity();
		
		// 速度ベクトルから回転角（Yaw, Pitch）を計算
		Vector3 velDir = { 0.0f, 0.0f, 1.0f }; // デフォルト
		float speed = Length(vel);
		Vector3 rotate = { 0.0f, 0.0f, 0.0f };
		if (speed > 0.001f) {
			velDir = Normalize(vel);
			rotate.y = std::atan2(velDir.x, velDir.z); // Yaw
			rotate.x = std::asin(-velDir.y);           // Pitch
		}

		// 曳光弾（Tracer）として、細長く引き伸ばす（Z軸方向）
		Vector3 scale = { 0.15f, 0.15f, 4.0f }; // 幅0.15m、長さ4mの線状メッシュ
		Vector4 color = { 1.0f, 0.2f, 0.1f, 1.0f };  // 赤色に発光

		PrimitiveModel::GetInstance()->DrawCylinder(
			scale, rotate, pos, color,
			texIndex, camera, BlendMode::kAdd
		);
	}

}

void BulletManager::SpawnBullet(const Vector3& position, const Vector3& direction, float speed, float damage) {
	// 非活性の弾丸スロットを探す
	for (auto& bullet : bullets_) {
		if (!bullet.IsAlive()) {
			bullet.Spawn(position, direction, speed, damage);
			return;
		}
	}
	// プール満杯の場合は発射されない（ドロップ）
}

uint32_t BulletManager::GetActiveBulletCount() const {
	uint32_t count = 0;
	for (const auto& bullet : bullets_) {
		if (bullet.IsAlive()) {
			++count;
		}
	}
	return count;
}
