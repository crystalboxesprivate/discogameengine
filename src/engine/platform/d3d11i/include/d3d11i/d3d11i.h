#pragma once

#include <graphicsinterface/graphicsinterface.h>
#include <d3d11.h>
#include <wrl.h>

using Microsoft::WRL::ComPtr;

namespace graphicsinterface {
// ID3D11Device *get_device();
// ID3D11DeviceContext *get_context();

static DXGI_FORMAT d3d_pixel_formats[] = {
    DXGI_FORMAT_R8G8B8A8_UNORM,     // R8G8B8A8F,
    DXGI_FORMAT_R32G32B32_FLOAT,    // R32G32B32F,
    DXGI_FORMAT_R32G32_FLOAT,       // R32G32F,
    DXGI_FORMAT_R32G32B32A32_FLOAT, // FloatRGBA,
    DXGI_FORMAT_UNKNOWN             // Unknown
};

struct D3D11Texture2D : public Texture2D {
  DXGI_FORMAT dxgi_format;
  ComPtr<ID3D11Texture2D> view;
  bool generate_mips = false;
  virtual void *get_native_ptr() {
    return view.Get();
  }
  PixelFormat pixel_format;
  u16 width, height;

  virtual u16 get_width() {
    return width;
  }
  virtual u16 get_height() {
    return height;
  }
  virtual PixelFormat get_pixel_format() {
    return pixel_format;
  }
};

struct D3D11ShaderResourceView : public ShaderResourceView {
  ComPtr<ID3D11ShaderResourceView> view;
  virtual void *get_native_ptr() {
    return view.Get();
  }
};

struct D3D11RenderTargetView : public RenderTargetView {
  ComPtr<ID3D11RenderTargetView> view;
  virtual void *get_native_ptr() {
    return view.Get();
  }
};

struct D3D11DepthStencilView : public DepthStencilView {
  ComPtr<ID3D11DepthStencilView> view;
  virtual void *get_native_ptr() {
    return view.Get();
  }
};

struct D3DVertexBuffer : public VertexBuffer {
  D3DVertexBuffer()
      : pixel_format(PixelFormat::Unknown) {
  }
  virtual void *get_native_resource() {
    return buffer.Get();
  }
  PixelFormat pixel_format;
  ComPtr<ID3D11Buffer> buffer = nullptr;
};
struct D3DIndexBuffer : public IndexBuffer {
  virtual void *get_native_resource() {
    return buffer.Get();
  }
  ComPtr<ID3D11Buffer> buffer = nullptr;
};

struct D3DUniformBuffer : public UniformBuffer {
  virtual usize get_count() {
    return size;
  }
  virtual usize get_stride() {
    return 1;
  }

  virtual void *get_native_resource() {
    return buffer.Get();
  }

  usize size;

  ComPtr<ID3D11Buffer> buffer = nullptr;
};
} // namespace graphicsinterface
