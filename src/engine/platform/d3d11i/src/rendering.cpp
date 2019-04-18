#pragma once

#include <d3d11i/d3d11i.h>
#include <d3d11i/d3dishader.h>

namespace graphicsinterface {
extern ID3D11Device *device;
extern ID3D11DeviceContext *device_context;
// hashmap contains as a key a combo of stream and vertex unique id
struct StreamShaderId {
  u32 shader_id;
  VertexStream stream;
  friend bool operator==(const StreamShaderId &lhs, const StreamShaderId &rhs) {
    bool equal = lhs.shader_id == rhs.shader_id && lhs.stream.count == rhs.stream.count;
    for (u8 x = 0; x < lhs.stream.count; x++)
      equal = equal && (lhs.stream.attributes[x].semantic_name == rhs.stream.attributes[x].semantic_name &&
                        lhs.stream.attributes[x].aligned_byte_offset == rhs.stream.attributes[x].aligned_byte_offset &&
                        lhs.stream.attributes[x].semantic_index == rhs.stream.attributes[x].semantic_index);
    return equal;
  }
  friend bool operator!=(const StreamShaderId &lhs, const StreamShaderId &rhs) {
    return !(lhs == rhs);
  }
};
} // namespace graphicsinterface

using graphicsinterface::StreamShaderId;
namespace std {
template <>
struct hash<StreamShaderId> {
  size_t operator()(const StreamShaderId &value) const {
    usize hash_value = value.stream.count;
    for (int x = 0; x < value.stream.count; x++) {
      hash_value += value.stream.attributes[x].aligned_byte_offset + value.stream.attributes[x].semantic_index +
                    (u8)value.stream.attributes[x].semantic_name;
    }
    return hash_value;
  }
};
} // namespace std

namespace graphicsinterface {
HashMap<StreamShaderId, ComPtr<ID3D11InputLayout>> input_layouts;

void bind_uniform_buffer(u32 slot, ShaderStage stage, UniformBufferRef uniform_buffer) {
  auto &buf = *(D3DUniformBuffer *)uniform_buffer.get();
  ID3D11Buffer *PerObjectBuffer = buf.buffer.Get();
  switch (stage) {
  case ShaderStage::Vertex:
    device_context->VSSetConstantBuffers(slot, 1, &PerObjectBuffer);
    break;
  case ShaderStage::Pixel:
    device_context->PSSetConstantBuffers(slot, 1, &PerObjectBuffer);
    break;
  default:
    assert(false && "Other stages to  be implemented.");
  }
}

struct D3D11SamplerState : public SamplerState {
  ComPtr<ID3D11SamplerState> view;
  virtual void *get_native_ptr() {
    return view.Get();
  }
};

SamplerStateRef create_sampler_state() {
  D3D11_SAMPLER_DESC samplerDesc;
  auto sampler_state = new D3D11SamplerState;
  // Create a texture sampler state description.
  samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
  samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
  samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
  samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
  samplerDesc.MipLODBias = 0.0f;
  samplerDesc.MaxAnisotropy = 1;
  samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
  samplerDesc.BorderColor[0] = 0;
  samplerDesc.BorderColor[1] = 0;
  samplerDesc.BorderColor[2] = 0;
  samplerDesc.BorderColor[3] = 0;
  samplerDesc.MinLOD = 0;
  samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

  // Create the texture sampler state.
  HRESULT result = device->CreateSamplerState(&samplerDesc, &sampler_state->view);
  if (FAILED(result)) {
    assert(false);
    return nullptr;
  }

  return SamplerStateRef(sampler_state);
}

void set_render_targets(i32 num_render_targets, RenderTargetViewRef *render_targets,
                        DepthStencilViewRef depth_stencil_view) {
  ID3D11RenderTargetView *d3d11_rtvs[D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT];
  for (int x = 0; x < num_render_targets; x++) {
    d3d11_rtvs[x] = (ID3D11RenderTargetView *)render_targets[x]->get_native_ptr();
  }
  device_context->OMSetRenderTargets(num_render_targets, d3d11_rtvs,
                                     depth_stencil_view ? (ID3D11DepthStencilView *)depth_stencil_view->get_native_ptr() : nullptr);
}

const char *semantic_names[] = {"POSITION", "TEXCOORD", "NORMAL", "TANGENT", "COLOR"};

namespace VertexBufferResources {
ID3D11Buffer *vertex_buffers[D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT];
u32 offsets[D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT];
u32 strides[D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT];
} // namespace VertexBufferResources

void set_vertex_stream(VertexStream &stream, ShaderRef VertexShader) {
  D3DVertexShader &vertex = *reinterpret_cast<D3DVertexShader *>(VertexShader.get());
  StreamShaderId id = {vertex.guid, stream};
  auto it = input_layouts.find(id);
  ComPtr<ID3D11InputLayout> layout;
  if (it == input_layouts.end()) {
    // Input layout
    D3D11_INPUT_ELEMENT_DESC layout_desc[VERTEX_ATTRIBUTE_COUNT_MAX];
    for (u32 x = 0; x < stream.count; x++) {
      auto &attr = stream.attributes[x];
      layout_desc[x] = {semantic_names[(u8)attr.semantic_name],
                        attr.semantic_index,
                        d3d_pixel_formats[(u8)attr.pixel_format],
                        attr.buffer_slot,
                        attr.aligned_byte_offset,
                        D3D11_INPUT_PER_VERTEX_DATA,
                        0u};
    }
    auto hr = device->CreateInputLayout(layout_desc, (u32)stream.count, vertex.blob.get_data(),
                                        (u32)vertex.blob.get_size(), &layout);
    if (FAILED(hr))
      assert(false);

    input_layouts[id] = layout;
  } else {
    layout = it->second;
  }

  assert(layout.Get());
  device_context->IASetInputLayout(layout.Get());
  {
    for (u32 x = 0; x < stream.buffer_count; x++) {
      auto buf = stream.buffers[x] != nullptr ? static_cast<D3DVertexBuffer *>(stream.buffers[x]) : nullptr;
      VertexBufferResources::vertex_buffers[x] = buf ? buf->buffer.Get() : nullptr;
      VertexBufferResources::strides[x] = buf ? (u32)buf->stride : 0;
      VertexBufferResources::offsets[x] = 0;
    }

    device_context->IASetVertexBuffers(0, (u32)stream.buffer_count, VertexBufferResources::vertex_buffers,
                                       VertexBufferResources::strides, VertexBufferResources::offsets);
  }
}

void set_pipeline_state(PipelineState &pipeline_state) {
  device_context->VSSetShader(
      reinterpret_cast<D3DVertexShader *>(pipeline_state.bound_shaders.vertex.get())->shader.Get(), 0, 0);
  device_context->PSSetShader(
      reinterpret_cast<ID3D11PixelShader *>(pipeline_state.bound_shaders.pixel->get_native_ptr()), 0, 0);

  D3D_PRIMITIVE_TOPOLOGY primitive_topology;
  switch (pipeline_state.primitive_type) {
  case PrimitiveTopology::PointList:
    primitive_topology = D3D11_PRIMITIVE_TOPOLOGY_POINTLIST;
    break;
  case PrimitiveTopology::LineList:
    primitive_topology = D3D11_PRIMITIVE_TOPOLOGY_LINELIST;
    break;
  case PrimitiveTopology::LineStrip:
    primitive_topology = D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP;
    break;
  case PrimitiveTopology::TriangleList:
    primitive_topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    break;
  case PrimitiveTopology::TriangleStrip:
    primitive_topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
    break;
  }
  device_context->IASetPrimitiveTopology(primitive_topology);
}

void draw(u32 vertex_count, u32 vertex_start_location) {
  device_context->Draw(vertex_count, vertex_start_location);
}

void draw_indexed(IndexBufferRef index_buffer, PrimitiveTopology primitive_type, u32 index_count, u32 start_index,
                  u32 vertex_start_index) {
  device_context->IASetIndexBuffer(reinterpret_cast<ID3D11Buffer *>(index_buffer->get_native_resource()),
                                   DXGI_FORMAT_R32_UINT, 0);

  device_context->DrawIndexed(index_count, start_index, vertex_start_index);
}
} // namespace graphicsinterface
