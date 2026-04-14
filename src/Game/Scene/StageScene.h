#include "IScene.h"
#include <vector>
#include "Object3d.h"
#include "Sprite.h"
#include "Skybox.h"
#include <memory> 

class StageScene : public IScene {
public:
	void Initialize() override;
	void Update() override;
	void Draw() override;
	void Finalize() override;
private:
	std::unique_ptr<Object3d> sphereObject = nullptr;
	std::unique_ptr<Object3d> terrainObject = nullptr;
	std::unique_ptr<Skybox> skybox_ = nullptr;
};