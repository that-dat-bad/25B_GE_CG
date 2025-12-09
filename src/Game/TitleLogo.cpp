#include "TitleLogo.h"
#include <cmath>
#include <numbers>
#include "Model.h"
#include "TDEngine.h"

TitleLogo::~TitleLogo() {
	if (tutorialObject3d_) delete tutorialObject3d_;
}

void TitleLogo::Initialize(const MyMath::Vector3& position) {
	std::string path = "./Resources/titleLogo/titleLogo.obj";
	std::string gamePath = "./Resources/gameLogo/gameLogo.obj";
	Model::LoadFromOBJ(path);
	tutorialObject3d_ = Object3d::Create();
	tutorialObject3d_->SetModel(path);
	tutorialObject3d_->SetTranslate(position);
	basePosition_ = position;
	Model::LoadFromOBJ(gamePath);
	gameObject3d_ = Object3d::Create();
	gameObject3d_->SetModel(gamePath);
	gameObject3d_->SetTranslate(position);
	gameBasePosition_ = position;
}

void TitleLogo::Update() {
	// ふわふわ動作
	theta_ += 3.14159265f / 60.0f;
	float offsetY = std::sin(theta_) * amplitude_;

	MyMath::Vector3 pos = basePosition_;
	pos.y += offsetY + 2.0f;

	if (TDEngine::GetInput()->triggerKey(DIK_W)) {
		isTutorial_ = true;
	}
	else if (TDEngine::GetInput()->triggerKey(DIK_S)) {
		isTutorial_ = false;
	}

	tutorialObject3d_->SetTranslate(pos);
	gameObject3d_->SetTranslate(pos);

	tutorialObject3d_->Update();
	gameObject3d_->Update();
}

void TitleLogo::Draw() {
	if (tutorialObject3d_ && gameObject3d_)
	{
		if (isTutorial_)
		{
			tutorialObject3d_->Draw();
		}
		else
		{
			gameObject3d_->Draw();
		}
	}
}