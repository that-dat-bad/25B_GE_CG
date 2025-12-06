#include "Skydome.h"
#include <cassert> // assert用

using namespace TDEngine;

void Skydome::Initialize(TDEngine::Model* model, TDEngine::Camera* camera) {
    assert(model);
    model_ = model;
    camera_ = camera;
    worldTransform_.Initialize();
}

void Skydome::Update() {

    worldTransform_.UpdateMatrix();
}

void Skydome::Draw() {
    model_->Draw(worldTransform_, *camera_);
}