#pragma once
#include <engine>

namespace graphicsinterface {
struct Buffer {
  inline usize get_num_bytes() {
    return get_count() * get_stride();
  }
  virtual usize get_count() = 0;
  virtual usize get_stride() = 0;
  virtual void *get_native_resource() = 0;
};

struct VertexBuffer : public Buffer {
  VertexBuffer()
      : count(0)
      , stride(0) {
  }

  usize count;
  usize stride;

  virtual usize get_count() {
    return count;
  }
  virtual usize get_stride() {
    return stride;
  }
};

struct IndexBuffer : public Buffer {
  IndexBuffer()
      : count(0) {
  }
  usize count;

  virtual usize get_count() {
    assert(count);
    return count;
  }
  virtual usize get_stride() {
    return sizeof(i32);
  }
};

typedef SharedPtr<IndexBuffer> IndexBufferRef;
typedef SharedPtr<VertexBuffer> VertexBufferRef;

struct UniformBuffer  : public Buffer {
};

typedef SharedPtr<UniformBuffer> UniformBufferRef;
UniformBufferRef create_uniform_buffer(usize size, void *initial_data);
void set_uniform_buffer_data(void *data, usize byte_count, UniformBufferRef buffer);

} // namespace graphicsinterface
