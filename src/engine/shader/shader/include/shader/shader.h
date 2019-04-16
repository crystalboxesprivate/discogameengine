#pragma once

#include <engine>
#include <asset/asset.h>
#include <graphicsinterface/shader.h>
#include <shader/compiler.h>
#include <shader/uniform_buffer.h>

namespace shader {
using asset::Asset;
using graphicsinterface::ShaderDescription;
using graphicsinterface::ShaderRef;
using graphicsinterface::ShaderStage;

struct Shader : public Asset {
  declare_asset_type(Shader);
  ShaderDescription description;

  Guid compile_id;

  Vector<UniformBufferDescription> uniform_buffers;
  ShaderRef compiled;

  virtual void serialize(Archive &archive) override {
    Asset::serialize(archive);
    archive << compile_id;
    archive << description;
    archive << uniform_buffers;
  }
  virtual void free() override {
    Asset::free();
    description.compiled_blob.clear();
    description.source.clear();
  }
};

bool get_path_from_virtual_path(const String &virtual_path, String &out_path);
Shader &load(const String &filename, ShaderStage stage, const String &entry);

} // namespace shader
