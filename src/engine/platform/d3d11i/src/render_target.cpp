#include <d3d11i/d3d11i.h>

namespace graphicsinterface {

RenderTargetViewRef create_render_target_view(usize width, usize height, PixelFormat pixelformat) {
  assert(false);
  return nullptr;
}

DepthStencilViewRef create_depth_stencil_view(usize width, usize height, PixelFormat pixelformat) {
  assert(false);
  return nullptr;
}

void clear_render_target_view(RenderTargetView &in_view, const glm::vec4 &clear_color) {
  ID3D11RenderTargetView *view = reinterpret_cast<ID3D11RenderTargetView *>(in_view.get_native_ptr());

  get_context()->ClearRenderTargetView(view, (float *)&clear_color[0]);
}
void clear_depth_stencil_view(DepthStencilView &in_view, u32 clear_flags, float depth, float stencil) {
  ID3D11DepthStencilView *view = reinterpret_cast<ID3D11DepthStencilView *>(in_view.get_native_ptr());
  get_context()->ClearDepthStencilView(view, clear_flags, depth, (u8)stencil);
}
} // namespace graphicsinterface
