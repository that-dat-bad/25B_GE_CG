#pragma once
#include "IScene.h"

class ClearScene : public IScene {
public:
    ClearScene();
    ~ClearScene();
    void Initialize() override;
    void Update() override;
    void Draw() override;
};

