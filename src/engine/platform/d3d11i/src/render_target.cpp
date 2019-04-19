#include <d3d11i/d3d11i.h>

namespace graphicsinterface {
extern ID3D11Device *device;
extern ID3D11DeviceContext *device_context;

i32 get_byte_count_from_pixelformat(PixelFormat pixel_format) {
  switch (pixel_format) {
  case PixelFormat::FloatRGBA:
    return 16;
  case PixelFormat::R32G32B32F:
    return 12;
  case PixelFormat::R32G32F:
    return 8;
  case PixelFormat::R8G8B8A8F:
    return 4;
  default:
    assert("Unimplemented" && false);
    break;
  }
  return 0;
}

Texture2DRef create_texture2d(usize width, usize height, PixelFormat pixelformat, void *data, bool generate_mips) {
  // TODO refactor this mess
  auto tex = new D3D11Texture2D;
  tex->width = (u16)width;
  tex->height = (u16)height;
  tex->pixel_format = pixelformat;
  tex->dxgi_format = d3d_pixel_formats[(u8)pixelformat];
  tex->generate_mips = generate_mips;

  D3D11_TEXTURE2D_DESC texture_desc;
  HRESULT result;
  // Setup the render target texture description.
  texture_desc.Width = tex->width;
  texture_desc.Height = tex->height;

  texture_desc.MipLevels = generate_mips ? 0 : 1;
  texture_desc.ArraySize = 1;
  texture_desc.Format = tex->dxgi_format;
  texture_desc.SampleDesc.Count = 1;
  texture_desc.SampleDesc.Quality = 0;
  texture_desc.Usage = D3D11_USAGE_DEFAULT;
  texture_desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
  texture_desc.CPUAccessFlags = 0;
  texture_desc.MiscFlags = generate_mips ? D3D11_RESOURCE_MISC_GENERATE_MIPS : 0;

  if (data) {
    i32 bytes_per_pixel = get_byte_count_from_pixelformat(pixelformat);
    i32 row_pitch = (u32)width * bytes_per_pixel;

    if (!generate_mips) {
      D3D11_SUBRESOURCE_DATA subresource_data;
      subresource_data.pSysMem = data;
      subresource_data.SysMemPitch = row_pitch;
      subresource_data.SysMemSlicePitch = (u32)(width * height) * bytes_per_pixel;
      result = device->CreateTexture2D(&texture_desc, &subresource_data, &tex->view);
    } else {
      result = device->CreateTexture2D(&texture_desc, nullptr, &tex->view);
      device_context->UpdateSubresource(tex->view.Get(), 0, NULL, data, row_pitch, 0);
    }
  } else {
    result = device->CreateTexture2D(&texture_desc, nullptr, &tex->view);
  }
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
  D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc{};

  // Setup the description of the shader resource view.
  shaderResourceViewDesc.Format = tex->dxgi_format;
  shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
  // shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
  shaderResourceViewDesc.Texture2D.MipLevels = -1;

  // Create the shader resource view.
  result = device->CreateShaderResourceView(tex->view.Get(), &shaderResourceViewDesc, &srv->view);
  if (FAILED(result)) {
    assert(false);
    return nullptr;
  }

  if (tex->generate_mips) {
    device_context->GenerateMips(srv->view.Get());
  }

  return ShaderResourceViewRef(srv);
}

ID3D11ShaderResourceView *shader_resource_views_arr[D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT];
void set_shader_resource_view(const ShaderParameter &shader_parameter, ShaderResourceViewRef srv, ShaderRef shader) {
  shader_resource_views_arr[0] = {nullptr};

  if (srv) {
    auto ptr = (ID3D11PixelShader *)shader->get_native_ptr();
    shader_resource_views_arr[0] = (ID3D11ShaderResourceView *)srv->get_native_ptr();
  }

  auto stage = shader->description.stage;
  if (stage == ShaderStage::Pixel) {
    device_context->PSSetShaderResources(shader_parameter.base_index, 1, shader_resource_views_arr);
    return;
  }
  assert(false && "Unsupported shader stage.");
}

ID3D11SamplerState *samplers[1];
void set_sampler_state(const ShaderParameter &shader_parameter, SamplerStateRef sampler_state, ShaderRef shader) {
  auto stage = shader->description.stage;
  if (sampler_state) {
    auto ptr = (ID3D11PixelShader *)shader->get_native_ptr();
    samplers[0] = (ID3D11SamplerState *)sampler_state->get_native_ptr();
  }
  if (stage == ShaderStage::Pixel) {
    device_context->PSSetSamplers(shader_parameter.base_index, 1, samplers);
    return;
  }
  assert(false && "Unsupported shader stage.");
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
  result = device->CreateRenderTargetView(tex->view.Get(), &desc, &rtv->view);
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

void clear_render_target_view(RenderTargetViewRef in_view, const glm::vec4 &clear_color) {
  ID3D11RenderTargetView *view = reinterpret_cast<ID3D11RenderTargetView *>(in_view->get_native_ptr());

  device_context->ClearRenderTargetView(view, (float *)&clear_color[0]);
}
void clear_depth_stencil_view(DepthStencilViewRef in_view, u32 clear_flags, float depth, float stencil) {
  ID3D11DepthStencilView *view = reinterpret_cast<ID3D11DepthStencilView *>(in_view->get_native_ptr());
  device_context->ClearDepthStencilView(view, clear_flags, depth, (u8)stencil);
}
} // namespace graphicsinterface
