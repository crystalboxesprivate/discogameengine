#pragma once

#include <renderer/resource.h>
#include <graphicsinterface/graphicsinterface.h>

namespace runtime {
struct EnvironmentMap;
struct EnvironmentMapResource : public renderer::Resource {
  EnvironmentMapResource(EnvironmentMap *in_texture)
      : owner(in_texture)
      , needs_to_init(true) {
  }
  
  graphicsinterface::SamplerStateRef sampler_state;

  graphicsinterface::TextureCubeRef color;
  graphicsinterface::ShaderResourceViewRef color_srv;

  graphicsinterface::TextureCubeRef irradiance;
  graphicsinterface::ShaderResourceViewRef irradiance_srv;

  graphicsinterface::TextureCubeRef prefilter;
  graphicsinterface::ShaderResourceViewRef prefilter_srv;

  virtual void init() override;
  virtual void release() override {
    assert(false && "hardware interface logic");
  }

  const EnvironmentMap *owner;
  bool needs_to_init;
};
} // namespace runtime
