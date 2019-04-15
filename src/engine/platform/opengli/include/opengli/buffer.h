#pragma once

#include <graphicsinterface/graphicsinterface.h>

namespace graphicsinterface {
struct GlVertexBuffer : public VertexBuffer {
  virtual void *get_native_resource() {
    return nullptr;
  }
  u32 vbo;
};
struct GlIndexBuffer : public IndexBuffer {
  virtual void *get_native_resource() {
    return nullptr;
  }
  u32 vbo;
};
} // namespace graphicsinterface
