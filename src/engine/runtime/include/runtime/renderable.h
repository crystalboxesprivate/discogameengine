#pragma once

namespace renderer {
struct Resource;
}

namespace runtime {
struct Renderable {
  virtual renderer::Resource *create_resource() = 0;
  virtual void init_resource() = 0;
  virtual void update_resource() = 0;
  virtual void release_resource() = 0;
  
  renderer::Resource *resource = nullptr;
};
} // namespace runtime
