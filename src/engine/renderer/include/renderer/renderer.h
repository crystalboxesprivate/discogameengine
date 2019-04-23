#pragma once

#include <renderer/screen_space_quad.h>
#include <renderer/post_processing.h>

namespace renderer {
struct Renderer {
  void initialize();
  void draw_image();

private:
  void load_shaders();
  void init_gbuffer();
  void draw_static_mesh_gbuffer();
  void draw_quad();

  void draw_light_pass();
  void draw_skybox();

  Quad quad;
  typedef SharedPtr<PostEffect> PostEffectRef;
  Vector<PostEffectRef> post_effects;
};

} // namespace renderer
