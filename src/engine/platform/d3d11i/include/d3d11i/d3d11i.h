#pragma once

#include <graphicsinterface/graphicsinterface.h>
#include <d3d11.h>
#include <wrl.h>

using Microsoft::WRL::ComPtr;

namespace graphicsinterface {
ID3D11Device *get_device();
ID3D11DeviceContext *get_context();

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
  D3DVertexBuffer() : pixel_format(PixelFormat::Unknown) {}
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
