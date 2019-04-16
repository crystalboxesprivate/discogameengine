#include <shader/cache.h>
#include <cross/cross.h>
#include <asset/asset.h>
#include <app/app.h>

using namespace asset;

namespace shader {
void ShaderCompiler::compile() {
  auto inputs = queue;
  queue.empty();

  Vector<Shader *> compiled;
  Vector<AssetRef> to_compile_shaders;
  Vector<shader::compiler::Input> to_compile_inputs;

  for (auto input : inputs) {
    String filename = input.virtual_source_path;
    String actual_path, alias = input.compile_id.to_string();
    shader::get_path_from_virtual_path(filename, actual_path);

    AssetRef new_shader = asset::add(actual_path, alias);
    assert(new_shader);

#if ENGINE_SHADER_FORCE_RECOMPILE
    asset::load_to_ram(new_shader, true);
#else
    asset::load_to_ram(new_shader);
#endif

    Shader &shader = *reinterpret_cast<Shader *>(new_shader.get());
    if (shader.description.needs_to_recompile) {
      to_compile_shaders.push_back(new_shader);
      to_compile_inputs.push_back(input);
    } else {
      compiled.push_back(&shader);
    }
  }

  Vector<shader::compiler::Output> compiler_outputs;
  Vector<String> out_sources;
  cross::cross_compile(to_compile_inputs, compiler_outputs, out_sources);
  {
    assert(inputs.size() == out_sources.size());
    for (i32 x = 0; x < inputs.size(); x++) {
      for (auto &err : compiler_outputs[x].errors) {
        DEBUG_LOG(Rendering, Error, "%s", err.shader_error.c_str());
      }
      DEBUG_LOG(Shaders, Log, "Compiled shader %s (%s)", inputs[x].virtual_source_path.c_str(),
                inputs[x].compile_id.to_string().c_str(), out_sources[x].data());
    }
  }
  for (i32 x = 0; x < to_compile_shaders.size(); x++) {
    Shader &shader = *reinterpret_cast<Shader *>(to_compile_shaders[x].get());
    shader.description.source = out_sources[x];
    shader.compile_id = to_compile_inputs[x].compile_id;
    String shader_code = shader.description.source;

    compiler::Output &out = compiler_outputs[x];

    for (compiler::ReflectionData::UniformBuffer &buffer_refl : out.reflection_data.uniform_buffers) {
      shader.uniform_buffers.resize(shader.uniform_buffers.size() + 1);
      UniformBufferDescription &buffer_desc = shader.uniform_buffers.back();
      buffer_desc.size = buffer_refl.size;
      initialize_uniform_buffer_members(buffer_desc, buffer_refl);
      buffer_desc.name += "_" + shader.compile_id.to_string();
    }

    shader.description.stage = (ShaderStage)to_compile_inputs[x].shader_stage;
    shader.description.source = shader_code;
    shader.description.entry_point = to_compile_inputs[x].entry_point;

    shader.compiled = graphicsinterface::create_shader(shader.description);

    DEBUG_LOG(Rendering, Log, "Saving shader to disk.");
    shader.description.needs_to_recompile = false;
    asset::resave_to_disk(to_compile_shaders[x]);
    compiled.push_back(&shader);
  }

  // CameraParameters ModelParameters
  ShaderCache &reg = *registry;
  auto &uniform_buffer_map = reg.uniform_buffer_map;

  for (auto &res : compiled) {
    auto &shader = *res;
    Vector<shader::UniformBufferDescription> new_buffers;
    for (auto &buf : shader.uniform_buffers) {
      using namespace utils::string;
      {
        const char *name = "CameraParameters";
        if (starts_with(buf.name, name)) {
          if (uniform_buffer_map.find(hash_code(name)) == uniform_buffer_map.end()) {
            uniform_buffer_map[hash_code(name)] = &buf;
            DEBUG_LOG(Rendering, Log, "Setting CameraParametersBuffer.");
          }
          continue;
        }
      }

      {
        const char *name = "ModelParameters";
        if (starts_with(buf.name, name)) {
          if (uniform_buffer_map.find(hash_code(name)) == uniform_buffer_map.end()) {
            uniform_buffer_map[hash_code(name)] = &buf;
            DEBUG_LOG(Rendering, Log, "Setting ModelParameters.");
          }
          continue;
        }
      }
      uniform_buffer_map[hash_code(buf.name)] = &buf;
      new_buffers.push_back(buf);
    }
    shader.uniform_buffers = new_buffers;
  }
}
} // namespace shader
