#include <renderer/screen_space_quad.h>

using namespace renderer;
namespace gi = graphicsinterface;

void Quad::initialize() {
  float quadVertices[] = {
      -1.0f, 1.0f,  0.0f, 0.0f, 0.0f, //
      1.0f,  1.0f,  0.0f, 1.0f, 0.0f, //
      -1.0f, -1.0f, 0.0f, 0.0f, 1.0f, //
      1.0f,  -1.0f, 0.0f, 1.0f, 1.0f,
  };
  vertices = gi::create_vertex_buffer(4, 20, gi::PixelFormat::Unknown, nullptr);
  gi::set_vertex_buffer_data(quadVertices, 80, vertices);

  stream.add({0, gi::SemanticName::Position, gi::PixelFormat::R32G32B32F, 0, 0});
  stream.add({0, gi::SemanticName::TexCoord, gi::PixelFormat::R32G32F, 12, 0});

  stream.add_buffer(vertices.get());
}
