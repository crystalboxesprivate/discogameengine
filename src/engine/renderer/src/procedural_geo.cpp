#include <renderer/procedural_geo.h>

namespace renderer {
// https://github.com/mrdoob/three.js/blob/master/src/geometries/SphereGeometry.js
void sphere_geometry(Vector<i32> &indices, Vector<glm::vec3> &vertices, f32 radius, i32 widthSegments,
                     i32 heightSegments, f32 phiStart, f32 phiLength, f32 thetaStart, f32 thetaLength) {
  using namespace glm;
  auto thetaEnd = thetaStart + thetaLength;

  i32 ix, iy;

  i32 index = 0;
  Vector<Vector<i32>> grid;

  vec3 vertex;

  for (iy = 0; iy <= heightSegments; iy++) {
    Vector<i32> verticesRow;
    f32 v = iy / (f32) heightSegments;
    // special case for the poles
    f32 uOffset = (iy == 0) ? 0.5f / (f32) widthSegments : ((iy == heightSegments) ? -0.5f / (f32)widthSegments : 0);
    for (ix = 0; ix <= widthSegments; ix++) {
      f32 u = ix / (f32) widthSegments;
      // vertex
      vertex.x = -radius * cos(phiStart + u * phiLength) * sin(thetaStart + v * thetaLength);
      vertex.y = radius * cos(thetaStart + v * thetaLength);
      vertex.z = radius * sin(phiStart + u * phiLength) * sin(thetaStart + v * thetaLength);
      vertices.push_back(vec3(vertex.x, vertex.y, vertex.z));
      verticesRow.push_back(index++);
    }

    grid.push_back(verticesRow);
  }

  // indices
  for (iy = 0; iy < heightSegments; iy++) {
    for (ix = 0; ix < widthSegments; ix++) {
      i32 a = grid[iy][ix + 1];
      i32 b = grid[iy][ix];
      i32 c = grid[iy + 1][ix];
      i32 d = grid[iy + 1][ix + 1];
      if (iy != 0 || thetaStart > 0) {
        indices.push_back(b);
        indices.push_back(a);
        indices.push_back(d);
      }
      if (iy != (heightSegments - 1 ) || thetaEnd < MATH_PI) {
        indices.push_back(b);
        indices.push_back(c);
        indices.push_back(d);
      }
    }
  }
}

void CreateSphere(int LatLines, int LongLines, Vector<i32> &indices, Vector<glm::vec3> &vertices) {
  using namespace glm;

  auto NumSphereVertices = ((LatLines - 2) * LongLines) + 2;
  auto NumSphereFaces = ((LatLines - 3) * (LongLines)*2) + (LongLines * 2);

  vertices.resize(NumSphereVertices);

  float sphereYaw = 0.0f;
  float spherePitch = 0.0f;

  vec3 currVertPos(0.0f, 0.0f, 1.0f);

  vertices[0].x = 0.0f;
  vertices[0].y = 0.0f;
  vertices[0].z = 1.0f;

  for (i32 i = 0; i < LatLines - 2; ++i) {
    spherePitch = (i + 1) * (3.14f / (LatLines - 1));
    mat4 Rotationx(1);
    Rotationx = rotate(Rotationx, spherePitch, vec3(1, 0, 0));

    for (i32 j = 0; j < LongLines; ++j) {
      sphereYaw = j * (6.28f / (LongLines));

      mat4 Rotationy(1);
      Rotationy = rotate(Rotationy, sphereYaw, vec3(0, 1, 0));

      vec4 currVertPos_vec4 = vec4(0.0f, 0.0f, 1.0f, 1.f) * (Rotationx * Rotationy);
      currVertPos = vec3(currVertPos_vec4.x, currVertPos_vec4.y, currVertPos_vec4.z);
      currVertPos = normalize(currVertPos);
      vertices[i * LongLines + j + 1] = currVertPos;
    }
  }

  vertices[NumSphereVertices - 1].x = 0.0f;
  vertices[NumSphereVertices - 1].y = 0.0f;
  vertices[NumSphereVertices - 1].z = -1.0f;

  indices.resize(NumSphereFaces * 3);

  int k = 0;
  for (i32 l = 0; l < LongLines - 1; ++l) {
    indices[k] = 0;
    indices[k + 1] = l + 1;
    indices[k + 2] = l + 2;
    k += 3;
  }

  indices[k] = 0;
  indices[k + 1] = LongLines;
  indices[k + 2] = 1;
  k += 3;

  for (i32 i = 0; i < LatLines - 3; ++i) {
    for (i32 j = 0; j < LongLines - 1; ++j) {
      indices[k] = i * LongLines + j + 1;
      indices[k + 1] = i * LongLines + j + 2;
      indices[k + 2] = (i + 1) * LongLines + j + 1;

      indices[k + 3] = (i + 1) * LongLines + j + 1;
      indices[k + 4] = i * LongLines + j + 2;
      indices[k + 5] = (i + 1) * LongLines + j + 2;

      k += 6; // next quad
    }

    indices[k] = (i * LongLines) + LongLines;
    indices[k + 1] = (i * LongLines) + 1;
    indices[k + 2] = ((i + 1) * LongLines) + LongLines;

    indices[k + 3] = ((i + 1) * LongLines) + LongLines;
    indices[k + 4] = (i * LongLines) + 1;
    indices[k + 5] = ((i + 1) * LongLines) + 1;

    k += 6;
  }

  for (i32 l = 0; l < LongLines - 1; ++l) {
    indices[k] = NumSphereVertices - 1;
    indices[k + 1] = (NumSphereVertices - 1) - (l + 1);
    indices[k + 2] = (NumSphereVertices - 1) - (l + 2);
    k += 3;
  }

  indices[k] = NumSphereVertices - 1;
  indices[k + 1] = (NumSphereVertices - 1) - LongLines;
  indices[k + 2] = NumSphereVertices - 2;
}

} // namespace renderer