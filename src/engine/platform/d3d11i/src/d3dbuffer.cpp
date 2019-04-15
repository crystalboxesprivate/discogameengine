#include <d3d11i/d3d11i.h>

namespace graphicsinterface {
HRESULT Result;

// TODO merge buffer creation code

UniformBufferRef create_uniform_buffer(usize size, void *initial_data) {
  D3DUniformBuffer &buffer = *new D3DUniformBuffer();
  buffer.size = size;

  D3D11_BUFFER_DESC desc;
  ZeroMemory(&desc, sizeof(desc));
  desc.Usage = D3D11_USAGE_DEFAULT;
  desc.ByteWidth = (u32)size;
  desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
  desc.CPUAccessFlags = 0;
  desc.MiscFlags = 0;
  Result = get_device()->CreateBuffer(&desc, nullptr, &buffer.buffer);

  if (FAILED(Result))
    assert(false && "Buffer creation failed");
  return UniformBufferRef(&buffer);
}

IndexBufferRef create_index_buffer(usize count, void *initial_data) {
  D3DIndexBuffer *buffer = new D3DIndexBuffer();
  buffer->count = count;

  D3D11_BUFFER_DESC desc;
  ZeroMemory(&desc, sizeof(desc));
  desc.Usage = D3D11_USAGE_DYNAMIC;
  desc.ByteWidth = (u32)buffer->get_num_bytes();
  desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
  desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
  Result = get_device()->CreateBuffer(&desc, (const D3D11_SUBRESOURCE_DATA *)initial_data, &buffer->buffer);

  if (FAILED(Result))
    assert(false && "Buffer creation failed");

  return IndexBufferRef(buffer);
}

VertexBufferRef create_vertex_buffer(usize count, usize element_size, PixelFormat pixel_format, void *initial_data) {
  D3DVertexBuffer &buffer = *new D3DVertexBuffer();
  buffer.count = count;
  buffer.stride = element_size;
  buffer.pixel_format = pixel_format;

  D3D11_BUFFER_DESC desc;
  ZeroMemory(&desc, sizeof(desc));
  desc.Usage = D3D11_USAGE_DYNAMIC;
  desc.ByteWidth = (u32)buffer.get_num_bytes();
  desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
  desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
  Result = get_device()->CreateBuffer(&desc, (const D3D11_SUBRESOURCE_DATA *)initial_data, &buffer.buffer);

  if (FAILED(Result))
    assert(false && "Buffer creation failed");

  return VertexBufferRef(&buffer);
}

void set_buffer_data(ID3D11Resource *resource, void *data, usize byte_count) {
  D3D11_MAPPED_SUBRESOURCE mapped_subresource;
  get_context()->Map(resource, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_subresource);
  memcpy(mapped_subresource.pData, data, byte_count);
  get_context()->Unmap(resource, 0);
}

void set_uniform_buffer_data(void *data, usize byte_count, UniformBufferRef buffer) {
  auto &uniform_buffer = *(D3DUniformBuffer *)buffer.get();
  get_context()->UpdateSubresource(uniform_buffer.buffer.Get(), 0, nullptr, data, 0, 0);
}

void set_index_buffer_data(void *data, usize byte_count, IndexBufferRef buffer) {
  assert(data);

  auto &index_buffer = *(D3DIndexBuffer *)buffer.get();

  set_buffer_data(index_buffer.buffer.Get(), data, byte_count);
}

void set_vertex_buffer_data(void *data, usize byte_count, VertexBufferRef buffer) {
  assert(data);
  auto &vertex_buffer = *(D3DVertexBuffer *)buffer.get();
  set_buffer_data(vertex_buffer.buffer.Get(), data, byte_count);
}
} // namespace graphicsinterface
