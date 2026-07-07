// 頂点データの入力構造体
struct VertexInput
{
    float32_t4 position : POSITION0;
    float2 texcoord : TEXCOORD0;
    float32_t3 normal : NORMAL0;
    float32_t4 weight : WEIGHT0;
    int4 indices : INDEX0;
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

struct Well {
    matrix skeletonSpaceMatrix;
    matrix skeletonSpaceInverseTransposeMatrix;
};

ConstantBuffer<TransformationMatrix> gTransformationMatrix : register(b1);
StructuredBuffer<Well> gMatrixPalette : register(t0);

VertexOutput main(VertexInput input)
{
    VertexOutput output;
    
    // スキニング行列の計算
    matrix skinnedMatrix =
        gMatrixPalette[input.indices.x].skeletonSpaceMatrix * input.weight.x +
        gMatrixPalette[input.indices.y].skeletonSpaceMatrix * input.weight.y +
        gMatrixPalette[input.indices.z].skeletonSpaceMatrix * input.weight.z +
        gMatrixPalette[input.indices.w].skeletonSpaceMatrix * input.weight.w;
        
    matrix skinnedNormalMatrix = 
        gMatrixPalette[input.indices.x].skeletonSpaceInverseTransposeMatrix * input.weight.x +
        gMatrixPalette[input.indices.y].skeletonSpaceInverseTransposeMatrix * input.weight.y +
        gMatrixPalette[input.indices.z].skeletonSpaceInverseTransposeMatrix * input.weight.z +
        gMatrixPalette[input.indices.w].skeletonSpaceInverseTransposeMatrix * input.weight.w;

    // スキニングによる変形
    float32_t4 skinnedPosition = mul(input.position, skinnedMatrix);
    float32_t3 skinnedNormal = mul(input.normal, (float32_t3x3)skinnedNormalMatrix);
    
    // 座標変換
    output.position = mul(skinnedPosition, gTransformationMatrix.WVP);
    
    // UV座標をそのまま渡す
    output.texcoord = input.texcoord;
    
    // 法線をワールド空間に変換して正規化
    output.normal = normalize(mul(skinnedNormal, (float32_t3x3) gTransformationMatrix.WorldInverseTranspose));
    
    // 頂点位置をワールド空間に変換
    output.worldPosition = mul(skinnedPosition, gTransformationMatrix.World).xyz;
    
    return output;
}
