#pragma once

#include <string>

struct CubemapBuilder {
  struct HDRI {
    int32_t width, height;
    int32_t id;
  };
  std::string filename;

  void init() {
    load_shaders();
    init_framebuffer();
    load_hdri();
  }
  void render_passes();

  int32_t environment_map;
  int32_t prefilter_map;
  int32_t irradiance_map;

private:
  void load_shaders();
  void init_framebuffer();
  void load_hdri();

  void save_to_disk();

  HDRI hdri;

  uint32_t capture_fbo;
  uint32_t capture_rbo;

};

void render_brdf_lut(uint16_t image_size);
