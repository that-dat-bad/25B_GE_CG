#include "StageScene.h"
#include <Novice.h>
#include <cmath>

void StageScene::Initialize() {


	constexpr int kScreenWidth = 1280;
	constexpr int kScreenHeight = 720;

	playerSize_ = 32;
	playerSpeed_ = 6.0f;
	playerX_ = kScreenWidth / 2.0f;
	playerY_ = kScreenHeight - 80.0f;

	enemySize_ = 48;
	enemyX_ = kScreenWidth / 2.0f;
	enemyY_ = 120.0f;
	enemyAlive_ = true;

	maxBullets_ = 64;
	bulletSpeed_ = 10.0f;
	bullets_.clear();
	bullets_.reserve(maxBullets_);

	sceneID = SCENE::STAGE;
}

void StageScene::Update() {
	// ESC でタイトル
	if (IScene::IsKeyTriggered(DIK_ESCAPE)) {
		sceneID = SCENE::TITLE;
		return;
	}

	if (keys[DIK_LEFT] != 0 || keys[DIK_A] != 0) {
		playerX_ -= playerSpeed_;
	}
	if (keys[DIK_RIGHT] != 0 || keys[DIK_D] != 0) {
		playerX_ += playerSpeed_;
	}

	constexpr int kHalfPad = 1;
	if (playerX_ < playerSize_ / 2.0f + kHalfPad) {
		playerX_ = playerSize_ / 2.0f + kHalfPad;
	}
	if (playerX_ > 1280.0f - playerSize_ / 2.0f - kHalfPad) {
		playerX_ = 1280.0f - playerSize_ / 2.0f - kHalfPad;
	}

	// 発射
	if (IScene::IsKeyTriggered(DIK_SPACE)) {
		bool spawned = false;
		for (auto& b : bullets_) {
			if (!b.active) {
				b.x = playerX_;
				b.y = playerY_ - playerSize_ / 2.0f - static_cast<float>(kBulletSize) / 2.0f;
				b.vy = -bulletSpeed_;
				b.active = true;
				spawned = true;
				break;
			}
		}
		if (!spawned && static_cast<int>(bullets_.size()) < maxBullets_) {
			Bullet nb;
			nb.x = playerX_;
			nb.y = playerY_ - playerSize_ / 2.0f - static_cast<float>(kBulletSize) / 2.0f;
			nb.vy = -bulletSpeed_;
			nb.active = true;
			bullets_.push_back(nb);
		}
	}

	// 弾の更新
	for (auto& b : bullets_) {
		if (!b.active) continue;
		b.y += b.vy;

		// 画面外に出たら無効化
		if (b.y < -static_cast<float>(kBulletSize)) {
			b.active = false;
			continue;
		}

		// 当たり判定
		if (enemyAlive_) {
			float halfB = static_cast<float>(kBulletSize) / 2.0f;
			float halfE = enemySize_ / 2.0f;
			bool hitX = std::fabs(b.x - enemyX_) <= (halfB + halfE);
			bool hitY = std::fabs(b.y - enemyY_) <= (halfB + halfE);
			if (hitX && hitY) {
				// ヒットしたらクリアシーンへ
				enemyAlive_ = false;
				sceneID = SCENE::CLEAR;
				return;
			}
		}
	}

}

void StageScene::Draw() {
	// プレイヤー（白四角）
	int px1 = static_cast<int>(playerX_ - playerSize_ / 2.0f);
	int py1 = static_cast<int>(playerY_ - playerSize_ / 2.0f);
	Novice::DrawBox(px1, py1, playerSize_, playerSize_, 0.0f, 0xFFFFFFFF, kFillModeSolid);

	// 敵（赤四角・動かない）
	if (enemyAlive_) {
		int ex1 = static_cast<int>(enemyX_ - enemySize_ / 2.0f);
		int ey1 = static_cast<int>(enemyY_ - enemySize_ / 2.0f);
		Novice::DrawBox(ex1, ey1, enemySize_, enemySize_, 0.0f, 0xFFFF00FF, kFillModeSolid);
	}

	// 弾
	for (const auto& b : bullets_) {
		if (!b.active) continue;
		int bx1 = static_cast<int>(b.x - static_cast<float>(kBulletSize) / 2.0f);
		int by1 = static_cast<int>(b.y - static_cast<float>(kBulletSize) / 2.0f);
		Novice::DrawBox(bx1, by1, kBulletSize, kBulletSize, 0.0f, 0xFFFFFFFF, kFillModeSolid);
	}

	// UI テキスト
	Novice::ScreenPrintf(0, 0, "Stage: Move Left/Right (A/D or <-/->), Shoot: SPACE");
}