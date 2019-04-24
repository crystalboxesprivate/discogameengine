#pragma once

#include <graphicsinterface/graphicsinterface.h>
#include <shader/shader.h>

namespace renderer {
struct GBuffer;
struct PostEffect {
  struct Parameters {};
  virtual void init() = 0;
  virtual void execute(graphicsinterface::ShaderResourceViewRef src, graphicsinterface::RenderTargetViewRef dst,
                       Parameters *parameters = nullptr, GBuffer *gbuffer = nullptr) = 0;
};

struct TonemapEffect : public PostEffect {
  struct TonemapParameters : Parameters {
    i32 mode = 0;
  };
  virtual void init() override;
  virtual void execute(graphicsinterface::ShaderResourceViewRef src, graphicsinterface::RenderTargetViewRef dst,
                       Parameters *parameters, GBuffer *gbuffer = nullptr) override;

  shader::Shader *tonemap_shader = nullptr;
};

struct MotionBlurEffect : public PostEffect {
  struct MotionBlurEffectParameters : Parameters {
  };
  virtual void init() override;
  virtual void execute(graphicsinterface::ShaderResourceViewRef src, graphicsinterface::RenderTargetViewRef dst,
                       Parameters *parameters, GBuffer *gbuffer = nullptr) override;

  graphicsinterface::SamplerStateRef clamp_sampler;

  shader::Shader *shader = nullptr;
};
} // namespace renderer
