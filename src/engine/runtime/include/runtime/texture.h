#pragma once

#include <runtime/renderable.h>

namespace runtime {
struct Texture : public Renderable {
  virtual usize get_size_x() = 0;
  virtual usize get_size_y() = 0;
};
} // namespace runtime
