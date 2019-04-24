#include <renderer/post_processing.h>
#include <renderer/screen_space_quad.h>
#include <renderer/gbuffer.h>

namespace gi = graphicsinterface;
using namespace renderer;

static const char *SHADER_TONEMAP_PATH = "/Shaders/post_effects/tonemap.hlsl";
static const char *SHADER_MOTION_BLUR_PATH = "/Shaders/post_effects/motionblur.hlsl";

void blit(gi::ShaderResourceViewRef src, gi::RenderTargetViewRef dst, shader::Shader* shader) {
  auto &quad = *screen_quad;
  gi::set_render_targets(1, &dst, nullptr);
  gi::clear_render_target_view(dst, glm::vec4(0));

  gi::PipelineState state;
  state.bound_shaders.vertex = quad.vertex->compiled;
  state.bound_shaders.pixel = shader->compiled;
  state.primitive_type = gi::PrimitiveTopology::TriangleStrip;
  
  gi::set_shader_resource_view({0}, src, shader->compiled);
  gi::set_sampler_state({0}, nullptr, shader->compiled);
  gi::set_pipeline_state(state);
  gi::set_vertex_stream(quad.stream, quad.vertex->compiled);
  gi::draw((u32)quad.vertices->count, 0);
  gi::set_shader_resource_view({0}, nullptr, shader->compiled);

}

void TonemapEffect::init() {
  tonemap_shader = &shader::load(SHADER_TONEMAP_PATH, gi::ShaderStage::Pixel, "PS");
}

void TonemapEffect::execute(gi::ShaderResourceViewRef src, gi::RenderTargetViewRef dst,
                            Parameters *parameters, GBuffer *gbuffer) {
  blit(src, dst, tonemap_shader);
}

void MotionBlurEffect::init() {
  shader = &shader::load(SHADER_MOTION_BLUR_PATH, gi::ShaderStage::Pixel, "PS");
  clamp_sampler = gi::create_sampler_state(gi::FILTER_MIN_MAG_MIP_LINEAR, gi::TEXTURE_ADDRESS_CLAMP);

}

void MotionBlurEffect::execute(gi::ShaderResourceViewRef src, gi::RenderTargetViewRef dst,
                            Parameters *parameters, GBuffer *gbuffer) {
  gi::set_sampler_state({0}, clamp_sampler, shader->compiled);
  gi::set_shader_resource_view({1}, gbuffer->motion_vectors.srv, shader->compiled);
  blit(src, dst, shader);
}
