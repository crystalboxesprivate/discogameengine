#pragma once

#include <engine>

namespace renderer {
#define MATH_PI 3.141592653589793f
void sphere_geometry(Vector<i32> &indices, Vector<glm::vec3> &vertices, f32 radius, i32 widthSegments,
                     i32 heightSegments, f32 phiStart = 0, f32 phiLength = MATH_PI * 2.f, f32 thetaStart = 0,
                     f32 thetaLength = MATH_PI);

void box_geometry(Vector<float> &vertices);

void CreateSphere(int LatLines, int LongLines, Vector<i32> &indices, Vector<glm::vec3> &position);
} // namespace renderer