#include <asset/shader_factory.h>
#include <shader/shader.h>
#include <config/config.h>
#include <utils/fs.h>

using shader::Shader;

const char *ShaderFactory::get_filename_extensions() {
  return "hlsl";
}

AssetRef ShaderFactory::create(const String &filename) {
  auto shader = new Shader;
  shader->description.source = utils::fs::load_file_to_string(filename);
  return AssetRef(shader);
}
