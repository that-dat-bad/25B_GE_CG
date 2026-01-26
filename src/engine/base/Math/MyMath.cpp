#include "MyMath.h"
#include <cmath>

namespace MyMath {

	float LerpShort(float a, float b, float t) {
		float diff = b - a;
		float PI = 3.14159265f;
		float twoPI = PI * 2.0f;

		// 角度差を -PI ~ +PI に補正
		// まず 0 ~ 2PI の範囲にする考え方もあるが、
		// 単純に while で回すのが確実でわかりやすい（最適化の余地はあるが）
		while (diff > PI) {
			diff -= twoPI;
		}
		while (diff < -PI) {
			diff += twoPI;
		}

		return a + diff * t;
	}

}
