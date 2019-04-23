#pragma once

#include <graphicsinterface/graphicsinterface.h>
#include <shader/shader.h>

namespace renderer {
struct PostEffect {
  struct Parameters {};
  virtual void init() = 0;
  virtual void execute(graphicsinterface::ShaderResourceViewRef src, graphicsinterface::RenderTargetViewRef dst,
                       Parameters *parameters = nullptr) = 0;
};

struct TonemapEffect : public PostEffect {
  struct TonemapParameters : Parameters {
    i32 mode = 0;
  };
  virtual void init() override;
  virtual void execute(graphicsinterface::ShaderResourceViewRef src, graphicsinterface::RenderTargetViewRef dst,
                       Parameters *parameters) override;

  shader::Shader *tonemap_shader = nullptr;
};
} // namespace renderer
