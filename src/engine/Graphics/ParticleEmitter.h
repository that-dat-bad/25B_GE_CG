#pragma once
#include <string>
#include <cstdint>
#include "../base/Math/MyMath.h"
using namespace MyMath;

#include "ParticleManager.h"

class ParticleEmitter {
public:
	ParticleEmitter(const std::string& name, const Transform& transform, uint32_t count, float frequency);

	void Update();

	void Emit();

	/// <summary>
	/// </summary>
	void SetParticleParameters(const ParticleParameters& params) { params_ = params; }

private:
	std::string name_;      
	Transform transform_;   
	uint32_t count_;        
	float frequency_;       
	float frequencyTime_;   

	ParticleParameters params_; 
};
