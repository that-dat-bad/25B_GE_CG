#include "Particle.hlsli"

struct VSInput
{
    float32_t4 position : POSITION;
    float32_t2 texcoord : TEXCOORD0;
};

VSOutput main(VSInput input, uint instanceId : SV_InstanceID)
{
    VSOutput output;

    ParticleForGPU particle = gParticleData[instanceId];

    output.svpos = mul(input.position, particle.WVP);
    
    output.texcoord = input.texcoord;
    
    output.color = particle.color;

    return output;
}
