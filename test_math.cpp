#include "src/engine/base/Math/MyMath.h"

using namespace MyMath;

void Test() {
    Vector3 v = {0,0,0};
    Matrix4x4 m = {};
    // Matrix initialization might be tricky with 2D array and simplified initializer lists
    
    WorldToScreen(v, m, m, 1280.0f, 720.0f);
}

