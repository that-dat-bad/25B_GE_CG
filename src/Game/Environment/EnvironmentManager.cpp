#include "EnvironmentManager.h"
#include <cmath>

void EnvironmentManager::Initialize() {
	timeOfDay_ = 17.0f; // 夕方（17時）から開始
	sun_.Initialize();
}

void EnvironmentManager::Update(Camera* camera, float deltaTime) {
	// 時間の進行 (仮の昼夜サイクル: リアル時間1秒につきゲーム内0.05時間進む)
	timeOfDay_ += deltaTime * 0.05f;
	if (timeOfDay_ > 24.0f) {
		timeOfDay_ = 0.0f;
	}

	// 太陽の回転計算 (時間角度に基づく円軌道)
	float angle = (timeOfDay_ / 24.0f) * 2.0f * 3.14159265f;
	
	// 太陽光の進行方向ベクトルを計算 (X-Y平面での円運動 + Z軸の傾き)
	MyMath::Vector3 dir;
	dir.x = std::cos(angle);
	dir.y = -std::sin(angle); // 昼間は下向き（-1.0f近く）、夜間は上向き
	dir.z = 0.3f;
	
	sun_.SetDirection(MyMath::Normalize(dir));

	// 太陽の高さ（地平線より上か下か）に応じて、明るさと色をコントロール
	// ※ライトの方向が下を向いている (dir.y < 0) 状態が、太陽が空にある (高い) 状態
	float sunHeight = -dir.y;
	if (sunHeight > 0.0f) {
		sun_.SetIntensity(sunHeight * 0.7f); // 高いほど明るい

		// 夕方・朝方 (高さ 0.0 〜 0.3) は赤みを強くする (サンセット効果)
		if (sunHeight < 0.3f) {
			float lerpFactor = sunHeight / 0.3f;
			sun_.SetColor({ 1.0f, 0.7f + 0.25f * lerpFactor, 0.4f + 0.45f * lerpFactor, 1.0f });
		} else {
			sun_.SetColor({ 1.0f, 0.95f, 0.85f, 1.0f });
		}
	} else {
		// 夜間は太陽光を0にする
		sun_.SetIntensity(0.0f);
	}

	// 太陽光設定の適用
	sun_.UpdateLight();

	// 太陽ブルーム（グレア）の動的適用
	sun_.UpdateBloom(camera);
}

void EnvironmentManager::Draw(Camera* camera) {
	// 太陽が地平線上にあるときのみ、太陽の見た目を描画する
	if (sun_.GetIntensity() > 0.0f) {
		sun_.Draw(camera);
	}
}
