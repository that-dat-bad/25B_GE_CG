// Object3D.PS.hlsl
// ピクセルシェーダー

struct PixelShaderOutput
{
    float32_t4 color : SV_TARGET;
};

// マテリアル情報を格納する定数バッファ
struct Material
{
    float32_t4 color;
    int enableLighting;
    matrix uvTransform;
    float32_t shininess;
};

// 平行光源の情報を格納する定数バッファ
struct DirectionalLight
{
    float32_t4 color;
    float32_t3 direction;
    float intensity;
};

// ライティングモデルを選択するための定数バッファ
struct LightingSettings
{
    int lightingModel; // 0: Lambert, 1: Half-Lambert
    float32_t3 padding;
};

struct Camera
{
    float32_t3 worldPosition;
};

ConstantBuffer<Material> gMaterial : register(b0);
ConstantBuffer<DirectionalLight> gDirectionalLight : register(b1);
ConstantBuffer<LightingSettings> gLightingSettings : register(b2);
ConstantBuffer<Camera> gCamera : register(b3);

Texture2D<float32_t4> gTexture : register(t0);
SamplerState gSampler : register(s0);

// 頂点シェーダーからの入力
struct PixelInput
{
    float32_t4 position : SV_POSITION;
    float32_t2 texcoord : TEXCOORD0;
    float32_t3 normal : NORMAL0;
    float32_t3 worldPosition : WORLDPOSITION0;
};

PixelShaderOutput main(PixelInput input)
{
    PixelShaderOutput output;

    float32_t4 texColor = float32_t4(1.0, 1.0, 1.0, 1.0); // UV無い時は白
    
    if (gMaterial.shininess >= 0.0f)
    {
        float32_t2 transformedUV = mul(float32_t4(input.texcoord, 0.0, 1.0), gMaterial.uvTransform).xy;
        texColor = gTexture.Sample(gSampler, transformedUV);
    }

    // マテリアルの色を乗算
    float32_t4 baseColor = texColor * gMaterial.color;
    output.color = baseColor;

    // ライティングが有効な場合
    if (gMaterial.enableLighting != 0)
    {
        float32_t3 normal = normalize(input.normal);
        float NdotL = dot(normal, -gDirectionalLight.direction);
        float cos = saturate(NdotL);
        
        float32_t3 diffuse = baseColor.rgb * gDirectionalLight.color.rgb * cos * gDirectionalLight.intensity;
        
        float32_t3 toEye = normalize(gCamera.worldPosition - input.worldPosition);
        float32_t3 reflectLight = reflect(gDirectionalLight.direction, normal);
        float RdotE = dot(reflectLight, toEye);
        float specularPow = pow(saturate(RdotE), gMaterial.shininess);
        //鏡面反射
        float32_t3 specular = gDirectionalLight.color.rgb * gDirectionalLight.intensity * specularPow * float32_t3(1.0f, 1.0f, 1.0f);

        // 光を反映
        output.color.rgb = diffuse + specular;
        output.color.a = texColor.a * gMaterial.color.a;
        
    }

    return output;
}
