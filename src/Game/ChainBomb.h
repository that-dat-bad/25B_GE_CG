#pragma once
#include "Object3d.h"
#include "Collision.h"
#include "EnemyDeathParticle.h"
#include <vector>
#include "AudioManager.h"

class Player;
class Enemy;

class ChainBomb {
public:
	~ChainBomb();
	void Initialize(const MyMath::Vector3& position);
	void Update();
	void Draw();

	// 連鎖爆発ロジック
	void ExplodeAround(const std::vector<ChainBomb*>& allChainBombs, float chainExplosionRadius);
	void Explode();

	// 衝突応答
	void OnCollision(const Player* player);
	void OnCollision(const Enemy* enemy);

	// ゲッター
	MyMath::Vector3 GetWorldPosition() const;
	AABB GetAABB();
	AABB GetAABB(float size); // 引数あり版（プレイヤーサイズ考慮など）

	bool IsExplode() const { return isExplode_; }
	bool IsDestroy() const { return isDestroy_; }

	void Restore();

private:
	Object3d* object3d_ = nullptr;
	MyMath::Vector3 size_ = { 2.0f, 2.0f, 2.0f };

	bool isExplode_ = false;
	bool isDestroy_ = false;

	static const int kExplodeFrame = 30;
	int explodeTimer_ = kExplodeFrame;

        // 復活用カウント（フレーム数）
        int reviveTimer_ = 0;
        const int kReviveFrame = 20 * 60; // 20秒

		// リスポーン演出用
        bool isRespawning_ = false;
        int respawnTimer_ = 0;
        static const int kRespawnEffectFrame = 60; // 1秒分くらいチカチカ

	EnemyDeathParticle* deathParticle_ = nullptr;

	// SE
	SoundData bombSe_;
	IXAudio2SourceVoice* pBgmVoice_ = nullptr;
};