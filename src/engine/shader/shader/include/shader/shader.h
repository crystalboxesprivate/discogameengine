#pragma once

#include <engine>
#include <asset/asset.h>
#include <shader/uniform_buffer.h>
#include <graphicsinterface/shader.h>

namespace shader {
using asset::Asset;
using graphicsinterface::ShaderDescription;
using graphicsinterface::ShaderRef;
using graphicsinterface::ShaderStage;

struct Shader : public Asset {
  declare_asset_type(Shader);
  ShaderDescription description;

  Vector<UniformBufferDescription> uniform_buffers;
  ShaderRef compiled;

  virtual void serialize(Archive &archive) override {
    Asset::serialize(archive);
    archive << description;
    archive << uniform_buffers;
  }
  virtual void free() override {
    Asset::free();
    description.compiled_blob.clear();
    description.source.clear();
  }
};

Shader &load(const String &filename, ShaderStage stage, const String &entry);

} // namespace shader
