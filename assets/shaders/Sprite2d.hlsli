struct VSOutput
{
    float32_t4 svpos : SV_POSITION;
    float32_t2 texcoord : TEXCOORD0;
};

struct Material
{
    float32_t4x4 WVP;
    float32_t4 color;
};

ConstantBuffer<Material> gMaterial : register(b0);
Texture2D<float32_t4> gTexture : register(t0);
SamplerState gSampler : register(s0);