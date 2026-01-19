#pragma once
#include "IScene.h"
#include <vector>

struct Bullet {
	float x;
	float y;
	float vy;
	bool active;
};

class StageScene : public IScene {
public:
	void Initialize() override;
	void Update() override;
	void Draw() override;

private:
	float playerX_;
	float playerY_;
	int playerSize_;
	float playerSpeed_;

	float enemyX_;
	float enemyY_;
	int enemySize_;
	bool enemyAlive_;

	std::vector<Bullet> bullets_;
	int maxBullets_;
	static constexpr int kBulletSize = 20;
	float bulletSpeed_;
};