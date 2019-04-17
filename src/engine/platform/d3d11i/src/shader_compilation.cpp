#pragma once

#include <d3d11i/d3d11i.h>
#include <d3d11i/d3dishader.h>

#include <d3dcompiler.h>
#pragma comment(lib, "d3dcompiler.lib")

namespace graphicsinterface {
extern ID3D11Device *device;
extern ID3D11DeviceContext *device_context;

ComPtr<ID3DBlob> ErrorBlob;

void compile_shader(ShaderDescription &shader) {
  ComPtr<ID3DBlob> blob;
  ComPtr<ID3DBlob> error_blob;
  DEBUG_LOG(Rendering, Log, "Recompiling from HLSL source...");

  switch (shader.stage) {
  case ShaderStage::Vertex: {
    HRESULT hr = D3DCompile(shader.source.c_str(), shader.source.size(), nullptr, nullptr, nullptr,
                            shader.entry_point.c_str(), "vs_5_0", D3DCOMPILE_ENABLE_STRICTNESS, 0, &blob, &error_blob);

    if (FAILED(hr)) {
      DEBUG_LOG(Shaders, Error, "%s", error_blob->GetBufferPointer());
      assert(false);
    }
    break;
  }
  case ShaderStage::Pixel: {
    HRESULT hr = D3DCompile(shader.source.c_str(), shader.source.size(), nullptr, nullptr, nullptr,
                            shader.entry_point.c_str(), "ps_5_0", D3DCOMPILE_ENABLE_STRICTNESS, 0, &blob, &error_blob);

    if (FAILED(hr)) {
      DEBUG_LOG(Shaders, Error, "%s", error_blob->GetBufferPointer());
      assert(false);
    }
    break;
  }
  default:
    DEBUG_LOG(Rendering, Log, "Unsupported shader stage %d", shader.stage);
    break;
  }
  shader.source.clear();
  shader.compiled_blob.resize(blob->GetBufferSize());
  memcpy(shader.compiled_blob.data(), blob->GetBufferPointer(), blob->GetBufferSize());
}

ShaderRef create_shader(ShaderDescription &shader) {
  assert(shader.stage != ShaderStage::NumStages);
  DEBUG_LOG(Rendering, Log, "Loading D3D shader");

  if (shader.needs_to_recompile) {
    assert(shader.source.size());
    assert(shader.entry_point.size());
    compile_shader(shader);
  }

  ShaderRef ref;
  D3DShader *d3d_shader = nullptr;
  switch (shader.stage) {
  case ShaderStage::Vertex: {

    auto vertex_shader = new D3DVertexShader();
    d3d_shader = vertex_shader;
    HRESULT hr = device->CreateVertexShader(shader.compiled_blob.data(), shader.compiled_blob.size(), nullptr,
                                                  &vertex_shader->shader);
    vertex_shader->blob.allocate(shader.compiled_blob.size(), shader.compiled_blob.data());
    if (FAILED(hr)) {
      assert(false);
    }
    ref = ShaderRef(vertex_shader);
    break;
  }
  case ShaderStage::Pixel: {
    auto pixel_shader = new D3DPixelShader();
    d3d_shader = pixel_shader;
    HRESULT hr = device->CreatePixelShader(shader.compiled_blob.data(), shader.compiled_blob.size(), nullptr,
                                                 &pixel_shader->shader);
    if (FAILED(hr)) {
      assert(false);
    }
    ref = ShaderRef(pixel_shader);
    break;
  }
  default:
    DEBUG_LOG(Rendering, Log, "Unsupported shader stage %d", shader.stage);
    break;
  }

  assert(d3d_shader);
  d3d_shader->description = shader;
  return ref;
}
} // namespace graphicsinterface
