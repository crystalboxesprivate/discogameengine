#pragma once

namespace renderer {
struct Renderer {
  void initialize();
  void draw_image();


  void init_gbuffer();
  void draw_gbuffer();
};
} // namespace renderer
