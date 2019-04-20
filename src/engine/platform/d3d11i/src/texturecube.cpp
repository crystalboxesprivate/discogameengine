#include <d3d11i/d3d11i.h>

namespace graphicsinterface {
extern ID3D11Device *device;
extern ID3D11DeviceContext *device_context;

// https://stackoverflow.com/a/34325668
struct D3D11TextureCube : public TextureCube {
  ComPtr<ID3D11Texture2D> view;
  D3D11TextureCube(u16 in_width, u16 in_height, PixelFormat in_pixel_format)
      : width(in_width)
      , height(in_height)
      , pixel_format(in_pixel_format) {
    assert(width);
  }

  virtual void *get_native_ptr() override {
    return view.Get();
  }
  virtual u16 get_width() override {
    return width;
  }
  virtual u16 get_height() override {
    return height;
  }
  D3D11_TEXTURE2D_DESC texArrayDesc;
  u16 width;
  u16 height;
  PixelFormat pixel_format;
  virtual PixelFormat get_pixel_format() override {
    return pixel_format;
  }
};

TextureCubeRef create_texture_cube(usize width, usize height, PixelFormat pixelformat, void *data) {
  assert(data);
  HRESULT result;
  auto dxgi_format = d3d_pixel_formats[(u8)pixelformat];
  i32 bytes_per_pixel = get_byte_count_from_pixelformat(pixelformat);
  i32 per_image_data_offset = (i32) (width * height) * bytes_per_pixel;
  i32 row_pitch = (u32)width * bytes_per_pixel;

  //
  ID3D11Texture2D *srcTex[6];
  ID3D11ShaderResourceView *srv[6];

  for (i32 x = 0; x < 6; x++) {
    D3D11_TEXTURE2D_DESC texture_desc;
    texture_desc.Width = (u32) width;
    texture_desc.Height = (u32) height;
    texture_desc.MipLevels = 0;
    texture_desc.ArraySize = 1;
    texture_desc.Format = dxgi_format;
    texture_desc.SampleDesc.Count = 1;
    texture_desc.SampleDesc.Quality = 0;
    texture_desc.Usage = D3D11_USAGE_DEFAULT;
    texture_desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
    texture_desc.CPUAccessFlags = 0;
    texture_desc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;
    result = device->CreateTexture2D(&texture_desc, nullptr, &srcTex[x]);
    if (FAILED(result)) {
      assert(false);
      return nullptr;
    }

    device_context->UpdateSubresource(srcTex[x], 0, NULL, (u8 *)data + per_image_data_offset * x, row_pitch, 0);

    // Setup the description of the shader resource view.
    D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc{};
    shaderResourceViewDesc.Format = dxgi_format;
    shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    shaderResourceViewDesc.Texture2D.MipLevels = -1;

    // Create the shader resource view.
    result = device->CreateShaderResourceView(srcTex[x], &shaderResourceViewDesc, &srv[x]);
    if (FAILED(result)) {
      assert(false);
      return nullptr;
    }

    device_context->GenerateMips(srv[x]);
  }

  auto tex = new D3D11TextureCube((u16)width, (u16)height, pixelformat);
  {
    tex->width = (u16)width;
    tex->height = (u16)height;
    tex->pixel_format = pixelformat;
  }

  // generate cubemap
  {
    D3D11_TEXTURE2D_DESC texElementDesc;
    ((ID3D11Texture2D *)srcTex[0])->GetDesc(&texElementDesc);

    D3D11_TEXTURE2D_DESC texArrayDesc;
    texArrayDesc.Width = texElementDesc.Width;
    texArrayDesc.Height = texElementDesc.Height;
    texArrayDesc.MipLevels = texElementDesc.MipLevels;
    texArrayDesc.ArraySize = 6;
    texArrayDesc.Format = texElementDesc.Format;
    texArrayDesc.SampleDesc.Count = 1;
    texArrayDesc.SampleDesc.Quality = 0;
    texArrayDesc.Usage = D3D11_USAGE_DEFAULT;
    texArrayDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    texArrayDesc.CPUAccessFlags = 0;
    texArrayDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

    // ID3D11Texture2D *texArray = 0;
    if (FAILED(device->CreateTexture2D(&texArrayDesc, 0, &tex->view))) {
      assert(false);
      return nullptr;
    }
    tex->texArrayDesc = texArrayDesc;

    // Copy individual texture elements into texture array.
    D3D11_BOX sourceRegion;

    // Here i copy the mip map levels of the textures
    for (UINT x = 0; x < 6; x++) {
      for (UINT mipLevel = 0; mipLevel < texArrayDesc.MipLevels; mipLevel++) {
        sourceRegion.left = 0;
        sourceRegion.right = (texArrayDesc.Width >> mipLevel);
        sourceRegion.top = 0;
        sourceRegion.bottom = (texArrayDesc.Height >> mipLevel);
        sourceRegion.front = 0;
        sourceRegion.back = 1;

        // test for overflow
        if (sourceRegion.bottom == 0 || sourceRegion.right == 0)
          break;

        device_context->CopySubresourceRegion(tex->view.Get(),
                                              D3D11CalcSubresource(mipLevel, x, texArrayDesc.MipLevels), 0, 0, 0,
                                              srcTex[x], mipLevel, &sourceRegion);
      }
    }
  }

  for (UINT x = 0; x < 6; x++) {
    srcTex[x]->Release();
    srv[x]->Release();
  }
  return TextureCubeRef(tex);
  // return it
}
ShaderResourceViewRef create_shader_resource_view(TextureCubeRef tex) {
  D3D11TextureCube *texturecube = (D3D11TextureCube *)tex.get();
  auto srv = new D3D11ShaderResourceView;

  // Create a resource view to the texture array.
  D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc;
  viewDesc.Format = texturecube->texArrayDesc.Format;
  viewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
  viewDesc.TextureCube.MostDetailedMip = 0;
  viewDesc.TextureCube.MipLevels = texturecube->texArrayDesc.MipLevels;

  if (FAILED(device->CreateShaderResourceView(texturecube->view.Get(), &viewDesc, &srv->view))) {
    assert(false);
    return nullptr;
  }
  return ShaderResourceViewRef(srv);
}
} // namespace graphicsinterface
