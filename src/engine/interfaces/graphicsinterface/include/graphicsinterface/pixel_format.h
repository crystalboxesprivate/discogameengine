#pragma once
#include <engine>
namespace graphicsinterface {
enum class PixelFormat : u8 {
  R8G8B8A8F,
  R32G32B32F,
  R32G32F,
  FloatRGBA,

  Unknown
};

static u8 get_byte_count_from_pixelformat(PixelFormat pixel_format) {
  switch (pixel_format) {
  case PixelFormat::FloatRGBA:
    return 16;
  case PixelFormat::R32G32B32F:
    return 12;
  case PixelFormat::R32G32F:
    return 8;
  case PixelFormat::R8G8B8A8F:
    return 4;
  default:
    assert("Unimplemented" && false);
    break;
  }
  return 0;
}

} // namespace graphicsinterface
