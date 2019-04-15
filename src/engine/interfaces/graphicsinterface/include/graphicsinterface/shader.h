#pragma once

#include <engine>

namespace graphicsinterface {
enum class ShaderStage : u32 {
  Vertex,
  Pixel,
  Geometry,
  Hull,
  Domain,
  Compute,

  NumStages,
};

inline Archive &operator<<(Archive &archive, ShaderStage &stage) {
  archive.serialize(&stage, sizeof(ShaderStage));
  return archive;
}

struct ShaderDescription {
  ShaderStage stage;
  String entry_point;
  String source;
  Vector<u8> compiled_blob;
  bool needs_to_recompile = true;
};

inline Archive &operator<<(Archive &archive, ShaderDescription &desc) {
  archive << desc.stage;
  archive << desc.entry_point;
  archive << desc.source;
  archive << desc.compiled_blob;
  archive << desc.needs_to_recompile;
  return archive;
}

struct Shader {
  ShaderDescription description;
  virtual void *get_native_ptr() = 0;
};

typedef SharedPtr<Shader> ShaderRef;
} // namespace graphicsinterface
