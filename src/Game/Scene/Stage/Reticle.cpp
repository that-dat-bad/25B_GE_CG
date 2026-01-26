#include "Reticle.h"

#include "TextureManager.h"

void Reticle::Initialize() {

	textureHandle_ = TextureManager::Load("reticle.png");
	
	sprite_ = std::make_unique<Sprite>(
	    textureHandle_, Vector2{0.0f, 0.0f}, Vector2{64.0f, 64.0f}, Vector4{0.0f, 1.0f, 0.0f, 1.0f}, Vector2{0.5f, 0.5f}, false, false 
	);
	sprite_->Initialize();
}

void Reticle::Update(const Vector3& targetWorldPos, const Camera& camera) {
	// 3D座標を2D画面座標に変換
	// mathStruct.h にあるか、MyMath にあるか確認が必要だが、一旦維持。
	// エラーが出るようなら別途修正する。
	position_ = WorldToScreen(targetWorldPos, camera.matView, camera.matProjection, 1280.0f, 720.0f);

	// スプライトに座標を適用
	sprite_->SetPosition(position_);
}

void Reticle::Draw() {
	if (sprite_) {
		sprite_->Draw();
	}
}