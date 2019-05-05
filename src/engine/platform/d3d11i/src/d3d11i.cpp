#include <d3d11i/d3d11i.h>
#include <ntwindow/ntwindow.h>
#include <config/config.h>

#include <d3d11.h>
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d11.lib")

namespace graphicsinterface {
ID3D11Device *device = nullptr;
ID3D11DeviceContext *device_context = nullptr;
IDXGISwapChain *swap_chain = nullptr;

D3D11RenderTargetView *render_target_view;
D3D11DepthStencilView *depth_stencil_view;
RenderTargetViewRef render_target_view_ref;
DepthStencilViewRef depth_stencil_view_ref;

ID3D11Texture2D *depth_stencil_buffer = nullptr;

void present() {
  swap_chain->Present(0, 0);
}

ID3D11Device *get_device() {
  return device;
}

ID3D11DeviceContext *get_context() {
  return device_context;
}

RenderTargetViewRef get_main_render_target_view() {
  return render_target_view_ref;
}

DepthStencilViewRef get_main_depth_stencil_view() {
  return depth_stencil_view_ref;
}

void *get_native_device() {
  return get_device();
}
void *get_native_context() {
  return get_context();
}

void create_rtv(i32 width, i32 height) {
  ID3D11Texture2D *BackBuffer;
  swap_chain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void **)&BackBuffer);

  render_target_view = new D3D11RenderTargetView;

  HRESULT hr = device->CreateRenderTargetView(BackBuffer, NULL, &render_target_view->view);
  if (FAILED(hr))
    assert(false);
  BackBuffer->Release();

  render_target_view_ref = RenderTargetViewRef(render_target_view);
}

ComPtr<ID3D11DepthStencilState> m_depthStencilState;
ComPtr<ID3D11DepthStencilState> m_depthDisabledStencilState;

void set_z_buffer(bool enabled) {
  device_context->OMSetDepthStencilState(enabled ? m_depthStencilState.Get() : m_depthDisabledStencilState.Get(), 1);
}

void inti_rasterizer_states() {
  HRESULT result;
  {
    D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
    ZeroMemory(&depthStencilDesc, sizeof(D3D11_DEPTH_STENCIL_DESC));

    // Set up the description of the stencil state.
    depthStencilDesc.DepthEnable = true;
    depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;

    depthStencilDesc.StencilEnable = true;
    depthStencilDesc.StencilReadMask = 0xFF;
    depthStencilDesc.StencilWriteMask = 0xFF;

    // Stencil operations if pixel is front-facing.
    depthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
    depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
    depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

    // Stencil operations if pixel is back-facing.
    depthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    depthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
    depthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
    depthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

    // Create the depth stencil state.
    result = device->CreateDepthStencilState(&depthStencilDesc, &m_depthStencilState);
    if (FAILED(result)) {
      assert(false);
    }
    // Set the depth stencil state.
    device_context->OMSetDepthStencilState(m_depthStencilState.Get(), 1);
  }
  {
    D3D11_DEPTH_STENCIL_DESC depthDisabledStencilDesc;
    // Clear the second depth stencil state before setting the parameters.
    ZeroMemory(&depthDisabledStencilDesc, sizeof(D3D11_DEPTH_STENCIL_DESC));

    // Now create a second depth stencil state which turns off the Z buffer for 2D rendering.  The only difference is
    // that DepthEnable is set to false, all other parameters are the same as the other depth stencil state.
    depthDisabledStencilDesc.DepthEnable = false;
    depthDisabledStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    depthDisabledStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;
    depthDisabledStencilDesc.StencilEnable = true;
    depthDisabledStencilDesc.StencilReadMask = 0xFF;
    depthDisabledStencilDesc.StencilWriteMask = 0xFF;
    depthDisabledStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    depthDisabledStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
    depthDisabledStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
    depthDisabledStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
    depthDisabledStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    depthDisabledStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
    depthDisabledStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
    depthDisabledStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

    // Create the state using the device.
    result = device->CreateDepthStencilState(&depthDisabledStencilDesc, &m_depthDisabledStencilState);
    if (FAILED(result)) {
      assert(false);
    }
  }
}

void create_dsv(i32 width, i32 height) {
  // depth stencil state
  inti_rasterizer_states();

  D3D11_TEXTURE2D_DESC DepthStencilDesc;
  DepthStencilDesc.Width = width;
  DepthStencilDesc.Height = height;
  DepthStencilDesc.MipLevels = 1;
  DepthStencilDesc.ArraySize = 1;
  DepthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
  DepthStencilDesc.SampleDesc.Count = 1;
  DepthStencilDesc.SampleDesc.Quality = 0;
  DepthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
  DepthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
  DepthStencilDesc.CPUAccessFlags = 0;
  DepthStencilDesc.MiscFlags = 0;
  depth_stencil_view = new D3D11DepthStencilView;

  device->CreateTexture2D(&DepthStencilDesc, NULL, &depth_stencil_buffer);
  device->CreateDepthStencilView(depth_stencil_buffer, NULL, &depth_stencil_view->view);

  depth_stencil_view_ref = DepthStencilViewRef(depth_stencil_view);
}

void init_viewport(i32 width, i32 height) {
  // Create the Viewport
  D3D11_VIEWPORT Viewport;
  ZeroMemory(&Viewport, sizeof(D3D11_VIEWPORT));
  Viewport.TopLeftX = 0;
  Viewport.TopLeftY = 0;
  Viewport.Width = (float)width;
  Viewport.Height = (float)height;

  Viewport.MinDepth = .0f;
  Viewport.MaxDepth = 1.f;

  device_context->RSSetViewports(1, &Viewport);
}

bool create_context(void *window_ptr, void *data) {
  window::Window &win = *reinterpret_cast<window::Window *>(window_ptr);
  i32 width, height;
  win.get_dimensions(width, height);

  DXGI_MODE_DESC BufferDesc;
  ZeroMemory(&BufferDesc, sizeof(DXGI_MODE_DESC));

  BufferDesc.Width = width;
  BufferDesc.Height = height;
  BufferDesc.RefreshRate.Numerator = 120;
  BufferDesc.RefreshRate.Denominator = 1;
  BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
  BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
  BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

  DXGI_SWAP_CHAIN_DESC SwapChainDesc;
  ZeroMemory(&SwapChainDesc, sizeof(DXGI_SWAP_CHAIN_DESC));

  SwapChainDesc.BufferDesc = BufferDesc;
  SwapChainDesc.SampleDesc.Count = 1;
  SwapChainDesc.SampleDesc.Quality = 0;
  SwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
  SwapChainDesc.BufferCount = 1;
  SwapChainDesc.OutputWindow = (HWND)win.get_native_window_ptr();
  SwapChainDesc.Windowed = TRUE;
  SwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

  u32 device_flag = 0;
#if ENGINE_DEBUG_D3D11LAYER
  device_flag |= D3D11_CREATE_DEVICE_DEBUG;
#endif
  D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, device_flag, NULL, NULL, D3D11_SDK_VERSION,
                                &SwapChainDesc, &swap_chain, &device, NULL, &device_context);

  create_rtv(width, height);
  create_dsv(width, height);

  ID3D11RenderTargetView *views[1];
  views[0] = render_target_view->view.Get();
  device_context->OMSetRenderTargets(1, views, depth_stencil_view->view.Get());
  init_viewport(width, height);
  // setWireFrameState();
  return true;
}

bool delete_context() {
  swap_chain->Release();
  device->Release();
  device_context->Release();

  // render_target_view->Release();
  // depth_stencil_view->Release();
  depth_stencil_buffer->Release();

  // WireFrameRasterizerState->Release();
  return true;
}
} // namespace graphicsinterface
