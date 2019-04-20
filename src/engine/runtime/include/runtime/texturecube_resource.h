#pragma once

#include <renderer/resource.h>
#include <graphicsinterface/graphicsinterface.h>

namespace runtime {
struct TextureCube;
struct TextureCubeResource : public renderer::Resource {
  TextureCubeResource(TextureCube *in_texture)
      : owner(in_texture)
      , needs_to_init(true) {
  }

  graphicsinterface::TextureCubeRef texcube_color;
  graphicsinterface::ShaderResourceViewRef srv_color;

  graphicsinterface::TextureCubeRef texcube_irradiance;
  graphicsinterface::ShaderResourceViewRef srv_irradiance;

  graphicsinterface::SamplerStateRef sampler_state;

  virtual void init() override;
  virtual void release() override {
    assert(false && "hardware interface logic");
  }

  const TextureCube *owner;
  bool needs_to_init;
};
} // namespace runtime
