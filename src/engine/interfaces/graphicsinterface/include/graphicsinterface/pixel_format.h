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
}
