#pragma once

#include <runtime/renderable.h>
#include <graphicsinterface/pixel_format.h>

namespace runtime {
struct Texture : public Renderable {
  virtual u16 get_size_x() const = 0;
  virtual u16 get_size_y() const = 0;
  virtual graphicsinterface::PixelFormat get_pixel_format() const = 0;
};
} // namespace runtime
