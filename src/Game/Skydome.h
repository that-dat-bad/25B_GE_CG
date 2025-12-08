#pragma once
#include "Object3d.h"

class Skydome {
public:
	~Skydome();
	void Initialize();
	void Update();
	void Draw();
private:
	Object3d* object3d_ = nullptr;
};