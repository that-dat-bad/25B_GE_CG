// 頂点データの入力構造体
struct VertexInput
{
    float32_t4 position : POSITION0;
    float2 texcoord : TEXCOORD0;
    float32_t3 normal : NORMAL0;
    float32_t4 weight : WEIGHT0;
    int4 indices : BLENDINDICES0;
};

// ピクセルシェーダーへの出力構造体
struct VertexOutput
{
    float32_t4 position : SV_POSITION;
    float2 texcoord : TEXCOORD0;
    float32_t3 normal : NORMAL0;
    float32_t3 worldPosition : WORLDPOSITION0;
};

// 変換行列を格納する定数バッファ
struct TransformationMatrix
{
    matrix WVP; // World * View * Projection
    matrix World; // World
    matrix WorldInverseTranspose; // WorldInverseTranspose for non-uniform scaling
};

ConstantBuffer<TransformationMatrix> gTransformationMatrix : register(b1);

struct BoneMatrix
{
    matrix matrices[100]; // 最大100ボーンの行列を格納
};

ConstantBuffer<BoneMatrix> gBones : register(b2);

VertexOutput main(VertexInput input)
{
    VertexOutput output;
    //4つのボーン行列を、ウェイト（影響度）の強さでブレンドする
    matrix skinnedMatrix =
        gBones.matrices[input.indices[0]] * input.weight.x +
        gBones.matrices[input.indices[1]] * input.weight.y +
        gBones.matrices[input.indices[2]] * input.weight.z +
        gBones.matrices[input.indices[3]] * input.weight.w;
    // ボーンの力で頂点座標を変形
    float32_t4 skinnedPosition = mul(input.position, skinnedMatrix);
    
    // 変形した座標に、カメラなどのWVP行列を掛けて画面に出力
    output.position = mul(skinnedPosition, gTransformationMatrix.WVP);
    
    // UV座標をそのまま渡す
    output.texcoord = input.texcoord;
    
    float32_t3 skinnedNormal = mul(input.normal, (float32_t3x3) skinnedMatrix);
    output.normal = normalize(mul(skinnedNormal, (float32_t3x3) gTransformationMatrix.WorldInverseTranspose));
    
    // 頂点位置をワールド空間に変換
    output.worldPosition = mul(skinnedPosition, gTransformationMatrix.World).xyz;
    return output;
}
