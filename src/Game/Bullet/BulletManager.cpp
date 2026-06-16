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

	// 弾丸テクスチャインデックス（白1x1テクスチャを使用）
	uint32_t texIndex = TextureManager::GetInstance()->GetTextureIndexByFilePath("assets/textures/white1x1.png");

	// アクティブな弾丸をPrimitiveModelの板ポリで描画（ビルボード風トレーサー）
	for (const auto& bullet : bullets_) {
		if (!bullet.IsAlive()) continue;

		Vector3 pos = bullet.GetPosition();
		Vector3 scale = { 0.5f, 0.5f, 0.5f };   // 小さな板ポリ
		Vector3 rotate = { 0.0f, 0.0f, 0.0f };
		Vector4 color = { 1.0f, 0.9f, 0.3f, 1.0f };  // 黄色のトレーサー

		PrimitiveModel::GetInstance()->DrawPlane(
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
