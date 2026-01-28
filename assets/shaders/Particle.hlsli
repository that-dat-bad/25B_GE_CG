struct VSOutput
{
    float32_t4 svpos : SV_POSITION;
    float32_t2 texcoord : TEXCOORD0;
    float32_t4 color : COLOR0;
};

struct ParticleForGPU
{
    float32_t4x4 WVP;
    float32_t4 color;
};

Texture2D<float32_t4> gTexture : register(t0);

StructuredBuffer<ParticleForGPU> gParticleData : register(t1);

SamplerState gSampler : register(s0);
