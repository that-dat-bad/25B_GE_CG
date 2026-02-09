#include "Reticle.h"

#include "TextureManager.h"
#include "SpriteCommon.h"

void Reticle::Initialize() {

	TextureManager::GetInstance()->LoadTexture("reticle.png");
	textureHandle_ = TextureManager::GetInstance()->GetTextureIndexByFilePath("reticle.png");
	
	SpriteCommon* spriteCommon = SpriteCommon::GetInstance();
	sprite_ = std::make_unique<Sprite>();
	sprite_->Initialize(spriteCommon, "reticle.png");
	sprite_->SetPosition({ 0.0f, 0.0f });
	sprite_->SetSize({ 64.0f, 64.0f });
	sprite_->SetColor({ 0.0f, 1.0f, 0.0f, 1.0f });
	sprite_->SetAnchorPoint({ 0.5f, 0.5f });
}

void Reticle::Update(const Vector3& targetWorldPos, const Camera& camera) {
	position_ = WorldToScreen(targetWorldPos, camera.GetViewMatrix(), camera.GetProjectionMatrix(), 1280.0f, 720.0f);

	sprite_->SetPosition(position_);
}

void Reticle::Draw() {
	if (sprite_) {
		sprite_->Update();
		sprite_->Draw();
	}
}
