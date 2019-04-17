#pragma once

namespace renderer {
struct Renderer {
  void initialize();

  void load_shaders();
  void draw_image();


  void init_gbuffer();
  void draw_gbuffer();
  void draw_quad();

  void draw_light_pass();
};
} // namespace renderer
