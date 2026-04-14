#include "Skybox.hlsli"

struct PixelShaderOutput {
    float32_t4 color : SV_TARGET;
};

// マテリアル情報
struct Material {
    float32_t4 color;
    int enableLighting;
    float shininess;
    float32_t2 padding;
    matrix uvTransform;
};

ConstantBuffer<Material> gMaterial : register(b0);

// Cubemapテクスチャ - Texture2Dではなく TextureCube を使用
TextureCube<float32_t4> gTexture : register(t0);
SamplerState gSampler : register(s0);

PixelShaderOutput main(VertexShaderOutput input) {
    PixelShaderOutput output;
    // cubemapから3次元方向ベクトル(texcoord)でサンプリング
    float32_t4 textureColor = gTexture.Sample(gSampler, input.texcoord);
    output.color = textureColor * gMaterial.color;
    return output;
}
