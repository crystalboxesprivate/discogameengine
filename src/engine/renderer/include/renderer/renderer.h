#pragma once

namespace renderer {
struct Renderer {
  void initialize();

  void load_shaders();
  void draw_image();


  void init_gbuffer();
  void draw_static_mesh_gbuffer();
  void draw_quad();

  void draw_light_pass();
  void draw_skybox();

};
} // namespace renderer
