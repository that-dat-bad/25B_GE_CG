#pragma once
#include <vector>
#include "PrimitiveModel.h"

class PrimitiveGenerator {
public:
	// リング（穴あきの円）の頂点データを生成
	// segments: 分割数, innerR: 内径, outerR: 外径
	static std::vector<PrimitiveModel::VertexData> CreateRing(int segments, float innerR, float outerR);

	// 円柱（シリンダー）の頂点データを生成
	// segments: 分割数, height: 高さ（原点から上下の長さ）, radius: 半径
	static std::vector<PrimitiveModel::VertexData> CreateCylinder(int segments, float height, float radius);
};
