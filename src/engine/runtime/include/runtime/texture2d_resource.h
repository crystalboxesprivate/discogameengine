#pragma once

#include <renderer/resource.h>
#include <graphicsinterface/graphicsinterface.h>

namespace runtime {
struct Texture2D;
struct Texture2DResource : public renderer::Resource {
  Texture2DResource(Texture2D *in_texture)
      : owner(in_texture)
      , needs_to_init(true) {
  }

  graphicsinterface::Texture2DRef texture2d;
  graphicsinterface::SamplerStateRef sampler_state;
  graphicsinterface::ShaderResourceViewRef srv;


  virtual void init() override;
  virtual void release() override {
    assert(false && "hardware interface logic");
  }

  const Texture2D *owner;
  bool needs_to_init;
};
} // namespace runtime
