#pragma once
class FlightModel {
public:
	void Initialize();
	void Update();

	//--アクセッサ--
	//-getter-


	//-setter-

private:
	float maxThrust_;//最大推力
	float maxMass_;//最大重量
	float FuelInternalCapacity_; //最大燃料搭載量(内部燃料タンクの容量)
};
