#pragma once

#include <graphicsinterface/graphicsinterface.h>

#define MAKE_MOTION_VECTOR_PASS 1

namespace game {
struct MaterialComponent;
}

namespace renderer {
struct RenderTargetResource {
  void initialize(i32 width, i32 height, graphicsinterface::PixelFormat pixel_format) {
    texture2d = graphicsinterface::create_texture2d(width, height, pixel_format);
    render_target_view = graphicsinterface::create_render_target_view(texture2d);
    srv = graphicsinterface::create_shader_resource_view(texture2d);
  }
  graphicsinterface::RenderTargetViewRef render_target_view;
  graphicsinterface::Texture2DRef texture2d;
  graphicsinterface::ShaderResourceViewRef srv;
};

struct GBuffer {
  RenderTargetResource world_pos;
  RenderTargetResource material_attributes;
  RenderTargetResource color;
  RenderTargetResource world_normal;

#if MAKE_MOTION_VECTOR_PASS
  RenderTargetResource motion_vectors;
  graphicsinterface::RenderTargetViewRef rts[5];
#else
  graphicsinterface::RenderTargetViewRef rts[4];
#endif

  graphicsinterface::SamplerStateRef sampler_state;

  void initialize();
  void draw_static_meshes();
  void draw_skinned_meshes();

  void set_material_parameters(game::MaterialComponent* material, graphicsinterface::PipelineState&state);


private:
  void pass_start(graphicsinterface::PipelineState&state, u32 vertex_type);
};
} // namespace renderer
