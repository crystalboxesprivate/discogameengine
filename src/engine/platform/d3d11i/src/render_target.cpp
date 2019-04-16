#include <d3d11i/d3d11i.h>

namespace graphicsinterface {

Texture2DRef create_texture2d(usize width, usize height, PixelFormat pixelformat) {
  auto tex = new D3D11Texture2D;
  tex->width = (u16)width;
  tex->height = (u16)height;
  tex->pixel_format = pixelformat;
  tex->dxgi_format = d3d_pixel_formats[(u8)pixelformat];

  D3D11_TEXTURE2D_DESC texture_desc;
  HRESULT result;
  // Setup the render target texture description.
  texture_desc.Width = tex->width;
  texture_desc.Height = tex->height;

  texture_desc.MipLevels = 1;
  texture_desc.ArraySize = 1;
  texture_desc.Format = tex->dxgi_format;
  texture_desc.SampleDesc.Count = 1;
  texture_desc.SampleDesc.Quality = 0;
  texture_desc.Usage = D3D11_USAGE_DEFAULT;
  texture_desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
  texture_desc.CPUAccessFlags = 0;
  texture_desc.MiscFlags = 0;

  result = get_device()->CreateTexture2D(&texture_desc, NULL, &tex->view);

  if (FAILED(result)) {
    assert(false);
    return nullptr;
  }
  return Texture2DRef(tex);
}

ShaderResourceViewRef create_shader_resource_view(Texture2DRef texture2d) {
  D3D11Texture2D *tex = reinterpret_cast<D3D11Texture2D *>(texture2d.get());
  auto srv = new D3D11ShaderResourceView;
  HRESULT result;
  D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;

  // Setup the description of the shader resource view.
  shaderResourceViewDesc.Format = tex->dxgi_format;
  shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
  shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
  shaderResourceViewDesc.Texture2D.MipLevels = 1;

  // Create the shader resource view.
  result = get_device()->CreateShaderResourceView(tex->view.Get(), &shaderResourceViewDesc, &srv->view);
  if (FAILED(result)) {
    assert(false);
  return nullptr;

  }

  return ShaderResourceViewRef(srv);
}

RenderTargetViewRef create_render_target_view(Texture2DRef texture2d) {
  D3D11Texture2D *tex = reinterpret_cast<D3D11Texture2D *>(texture2d.get());
  auto rtv = new D3D11RenderTargetView;

  HRESULT result;
  D3D11_RENDER_TARGET_VIEW_DESC desc;

  desc.Format = tex->dxgi_format;
  desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
  desc.Texture2D.MipSlice = 0;

  // Create the render target view.
  result = get_device()->CreateRenderTargetView(tex->view.Get(), &desc, &rtv->view);
  if (FAILED(result)) {
    assert(false);
    return nullptr;
  }
  return RenderTargetViewRef(rtv);
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
