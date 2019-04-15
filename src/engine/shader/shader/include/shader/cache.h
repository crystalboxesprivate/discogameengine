#pragma once
#include <shader/uniform_buffer.h>

namespace shader {
struct ShaderCache {
  
  HashMap<String, UniformBufferDescription*> uniform_buffer_map;
};
} // namespace shader
