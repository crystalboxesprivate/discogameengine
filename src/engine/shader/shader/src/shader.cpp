#include <shader/shader.h>
#include <cross/cross.h>
#include <asset/asset.h>
#include <shader/uniform_buffer.h>
#include <graphicsinterface/buffer.h>
#include <graphicsinterface/graphicsinterface.h>
#include <app/app.h>
#include <shader/cache.h>
#include <utils/string.h>
#include <config/config.h>

namespace shader {
using namespace cross;
using namespace asset;

const char *shader_stage_to_string(ShaderStage stage) {
  switch (stage) {
  default:
    return "";
  case ShaderStage::Vertex:
    return "Vertex";
  case ShaderStage::Pixel:
    return "Pixel";
  case ShaderStage::Geometry:
    return "Geometry";
  case ShaderStage::Hull:
    return "Hull";
  case ShaderStage::Domain:
    return "Domain";
  case ShaderStage::Compute:
    return "Compute";
  };
}

Shader &load(const String &filename, ShaderStage stage, const String &entry) {
  DEBUG_LOG(Rendering, Log, "Loading %s shader %s, entry: %s:", shader_stage_to_string(stage), filename.c_str(),
            entry.c_str());

  AssetRef new_shader = asset::add(filename, shader_stage_to_string(stage));
  assert(new_shader);

#if ENGINE_SHADER_FORCE_RECOMPILE
  asset::load_to_ram(new_shader, true);
#else
  asset::load_to_ram(new_shader);
#endif

  Shader &shader = *reinterpret_cast<Shader *>(new_shader.get());

  if (shader.description.needs_to_recompile) {
    DEBUG_LOG(Rendering, Log, "Running cross compilation.");
    Vector<compiler::Output> outputs;

    Vector<compiler::Input> inputs = {{new_shader->source_filename, entry, (cross::Stage)stage, "", {}, false}};
    Vector<String> out_sources;
    cross_compile(inputs, outputs, out_sources);

    compiler::Output &out = outputs[0];

    for (compiler::Error &err : out.errors) {
      String error_string = err.shader_name +  ": " + err.shader_error;
      char s[512];
      sprintf(s, "%s", error_string.c_str());
      DEBUG_LOG(System, Error, "%s", s);
    }

    String shader_code = config::SHADER_TARGET_LANGUAGE != String("hlsl") ? out_sources[0] : shader.description.source;

    for (compiler::ReflectionData::UniformBuffer &buffer_refl : out.reflection_data.uniform_buffers) {
      shader.uniform_buffers.resize(shader.uniform_buffers.size() + 1);
      UniformBufferDescription &buffer_desc = shader.uniform_buffers.back();
      buffer_desc.size = buffer_refl.size;
      initialize_uniform_buffer_members(buffer_desc, buffer_refl);
    }

    shader.description.stage = (ShaderStage)stage;
    shader.description.source = shader_code;
    shader.description.entry_point = entry;
  } else {
    DEBUG_LOG(Rendering, Log, "Loaded from cache.");
  }

  auto &uniform_buffer_map = app::get().get_shader_cache().uniform_buffer_map;
  for (auto &buf : shader.uniform_buffers) {
    uniform_buffer_map[buf.name] = &buf;
  }

  shader.compiled = graphicsinterface::create_shader(shader.description);
  assert(shader.compiled);

  if (shader.description.needs_to_recompile) {
    DEBUG_LOG(Rendering, Log, "Saving shader to disk.");
    shader.description.needs_to_recompile = false;
    asset::resave_to_disk(new_shader);
  }
  // Delete source and shader blob as it's not needed anymore.
  new_shader->free();
  return shader;
}

implement_asset_type(Shader);
} // namespace shader
