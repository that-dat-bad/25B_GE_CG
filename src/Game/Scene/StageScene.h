#include "IScene.h"
#include <vector>
#include "Object3d.h"
#include "Sprite.h"
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
};