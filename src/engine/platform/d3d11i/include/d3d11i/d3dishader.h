#pragma once

#include <d3d11i/d3d11i.h>

namespace graphicsinterface {
struct D3DShader : public Shader {
  D3DShader()
      : guid(Guid::make_new().a) {
  }
  u32 guid;
};

struct D3DVertexShader : public D3DShader {
  Blob blob; // this one is used for creating input layouts.
  ComPtr<ID3D11VertexShader> shader;

  virtual void *get_native_ptr() override {
    return shader.Get();
  }
};

struct D3DPixelShader : public D3DShader {
  ComPtr<ID3D11PixelShader> shader;

  virtual void *get_native_ptr() override {
    return shader.Get();
  }
};
} // namespace graphicsinterface
