#pragma once

#include <graphicsinterface/graphicsinterface.h>

namespace shader {
struct Shader;
}

namespace renderer {
struct Quad {
  graphicsinterface::VertexBufferRef vertices;
  graphicsinterface::VertexStream stream;

  shader::Shader *vertex = nullptr;
  shader::Shader *pixel = nullptr;

  void initialize();
};
extern Quad *screen_quad;
} // namespace renderer
