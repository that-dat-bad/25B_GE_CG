#include "ClearScene.h"
#include "ImguiManager.h"
#include "Object3dCommon.h"
#include "TextureManager.h"
#include "Sprite.h"
#include "SpriteCommon.h"
#include <string>

using namespace MyMath;

bool ClearScene::isWin = false;
int ClearScene::finalScore = 0;

ClearScene::~ClearScene() {
	delete winSprite_;
	delete loseSprite_;
	for (int i = 0; i < 10; i++) {
		delete numberSprites_[i];
	}
}

void ClearScene::Initialize() {
	input_ = Input::GetInstance();
	camera_.SetTranslate({ 0.0f, 0.0f, -10.0f });

	SpriteCommon* spriteCommon = SpriteCommon::GetInstance();
	TextureManager* texManager = TextureManager::GetInstance();

	// Win/Lose Logic
	if (finalScore >= 2500) {
		isWin = true;
	} else {
		isWin = false;
	}

	// Load Win/Lose Textures
	texManager->LoadTexture("youwin.png");
	texManager->LoadTexture("youlose.png");

	winSprite_ = new Sprite();
	winSprite_->Initialize(spriteCommon, "youwin.png");
	winSprite_->SetPosition({ 640.0f, 200.0f });
	winSprite_->SetSize({ 600.0f, 150.0f }); // Adjust size as needed
	winSprite_->SetAnchorPoint({ 0.5f, 0.5f });

	loseSprite_ = new Sprite();
	loseSprite_->Initialize(spriteCommon, "youlose.png");
	loseSprite_->SetPosition({ 640.0f, 200.0f });
	loseSprite_->SetSize({ 600.0f, 150.0f }); // Adjust size as needed
	loseSprite_->SetAnchorPoint({ 0.5f, 0.5f });


	// Load Number Textures (0.png - 9.png)
	for (int i = 0; i < 10; i++) {
		std::string name = std::to_string(i) + ".png";
		texManager->LoadTexture(name);
		
		numberSprites_[i] = new Sprite();
		numberSprites_[i]->Initialize(spriteCommon, name);
		numberSprites_[i]->SetSize({ 32.0f, 64.0f }); // Adjust digit size
		numberSprites_[i]->SetAnchorPoint({ 0.0f, 0.0f });
	}

}

void ClearScene::Finalize() {
}

std::optional<SceneID> ClearScene::Update() {
	if (input_->TriggerKey(DIK_SPACE)) {
		return SceneID::kTitle;
	}
	return std::nullopt;
}

void ClearScene::Draw() {
	DirectXCommon* dxCommon = DirectXCommon::GetInstance();
	Object3dCommon::GetInstance()->SetupCommonState();

	SpriteCommon::GetInstance()->SetupCommonState();

	// Draw Result
	if (isWin) {
		if (winSprite_) winSprite_->Draw();
	} else {
		if (loseSprite_) loseSprite_->Draw();
	}

	// Draw Score
	std::string scoreStr = std::to_string(finalScore);
	// Center the score: calculate total width
	float digitWidth = 32.0f;
	float startX = 640.0f - (scoreStr.length() * digitWidth) / 2.0f;
	float startY = 400.0f;

	for (size_t i = 0; i < scoreStr.length(); i++) {
		int digit = scoreStr[i] - '0';
		if (digit >= 0 && digit <= 9 && numberSprites_[digit]) {
			numberSprites_[digit]->SetPosition({ startX + i * digitWidth, startY });
			numberSprites_[digit]->Update(); // Update transform
			numberSprites_[digit]->Draw();
		}
	}
}
