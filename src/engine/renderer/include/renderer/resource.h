#pragma once

namespace renderer {
  struct Resource {
    virtual void init() = 0;
    virtual void release() = 0;
    virtual void* get_graphics_resource() = 0;
  };
} // namespace renderer
