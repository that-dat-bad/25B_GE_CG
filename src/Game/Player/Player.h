#pragma once
class Player {
public:
	void Initialize();
	void Update();
	void Draw();
	void Finalize();

	//--アクセッサ--
	//-getter-


	//-setter-

private:
	float currentThrottle_; //ユーザー入力によるスロットル値
	float currentThrust_;
	float currentMass_;
	float currentFuel_; //残燃料
};

