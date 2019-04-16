#pragma once
#include <shader/uniform_buffer.h>
#include <graphicsinterface/graphicsinterface.h>

namespace shader {
struct ShaderCache;
struct ShaderCompiler {
  ShaderCompiler(ShaderCache *in_registry)
      : registry(in_registry) {
  }
  void add(shader::compiler::Input &input) {
    queue.push_back(input);
  }

  void add_multiple(Vector<shader::compiler::Input> &inputs) {
    for (auto &input : inputs) {
      queue.push_back(input);
    }
  }

  void compile();

  Vector<shader::compiler::Input> queue;
  ShaderCache *registry;
};

struct ShaderCache {
  ShaderCache()
      : compiler(this) {
  }

  ShaderCompiler compiler;
  HashMap<Guid, graphicsinterface::ShaderRef> shaders;
  HashMap<usize, UniformBufferDescription *> uniform_buffer_map;
};
} // namespace shader
