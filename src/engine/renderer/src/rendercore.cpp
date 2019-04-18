#include <renderer/rendercore.h>
#include <app/app.h>
#include <shader/cache.h>
#include <config/config.h>

static Vector<RenderPass *> render_pass_types;
static Vector<VertexType *> vertex_types;

namespace renderer {
void initialize_default_shaders() {
  Vector<shader::compiler::Input> inputs;
  for (auto &render_pass_type : render_pass_types) {
    render_pass_type->get_compile_jobs(inputs);
  }
}
} // namespace renderer

void RenderPass::preprocess_vertex_type(const String &filename, shader::compiler::Environment &env) {
  shader::compiler::Input input;
  shader::compiler::Output output;
  input.virtual_source_path = filename;
  preprocessor::run(input.source, output, input);
  env.virtual_path_to_contents["/Shaders/VertexType.generated.hlsl"] = input.source;
  for (auto &error : output.errors) {
    DEBUG_LOG(Rendering, Error, "%s: %s", error.shader_name.c_str(), error.shader_error.c_str());
  }
}

asset::AssetHandle<MaterialShader> MaterialShader::add(const String &filename) {
  String actual_path;
  shader::get_path_from_virtual_path(filename, actual_path);
  asset::AssetHandle<MaterialShader> handle = asset::add<MaterialShader>(actual_path, "MaterialShader");
  auto asset = handle.get();
  asset::load_to_ram(handle.get_ref());

  for (auto &vertex_type : vertex_types) {
    shader::compiler::Environment env;
    RenderPass::preprocess_vertex_type(vertex_type->get_filename(), env);

    Guid vertex_guid;
    Guid pixel_guid;

    if (asset->compiled_shaders.find(vertex_type->get_unique_id()) != asset->compiled_shaders.end()) {
      auto &pair = asset->compiled_shaders[vertex_type->get_unique_id()];
      assert(pair.vertex.global_id.is_valid() && pair.pixel.global_id.is_valid());
      vertex_guid = pair.vertex.global_id;
      pixel_guid = pair.pixel.global_id;
      
#if ENGINE_SHADER_FORCE_RECOMPILE
      shader::compiler::Input input;
      shader::compiler::Output output;
      input.virtual_source_path = filename;
      preprocessor::run(input.source, output, input);
      env.virtual_path_to_contents["/Shaders/UserShader.generated.hlsl"] = input.source;
#endif
    } else {
      shader::compiler::Input input;
      shader::compiler::Output output;
      input.virtual_source_path = filename;
      preprocessor::run(input.source, output, input);
      env.virtual_path_to_contents["/Shaders/UserShader.generated.hlsl"] = input.source;

      vertex_guid = Guid::make_new();
      pixel_guid = Guid::make_new();
    }
    asset->compiled_shaders[vertex_type->get_unique_id()] = {{vertex_guid, nullptr}, {pixel_guid, nullptr}};

    String gbuffer = GBufferRenderPass::get_filename_static();
    shader::compiler::Input input_vert = {gbuffer, "VSMain", shader::ShaderStage::Vertex, "", "", vertex_guid,
                                          env,     false};
    shader::compiler::Input input_pixel = {gbuffer, "PSMain", shader::ShaderStage::Pixel, "", "", pixel_guid,
                                           env,     false};

    app::get().get_shader_cache().compiler.add(input_vert);
    app::get().get_shader_cache().compiler.add(input_pixel);
  }
  asset::resave_to_disk(handle.get_ref());
  return handle;
}

void ShadowMapRenderPass::get_compile_jobs(Vector<shader::compiler::Input> &inputs) {
  for (auto &vertex_type : vertex_types) {
    shader::compiler::Environment env;
    RenderPass::preprocess_vertex_type(vertex_type->get_filename(), env);

    auto vertex_guid = Guid::make_new();
    auto pixel_guid = Guid::make_new();

    compiled_shaders[vertex_type->get_unique_id()] = {{vertex_guid, nullptr}, {pixel_guid, nullptr}};

    Vector<String> output_sources;
    inputs.push_back({get_filename(), "VSMain", shader::ShaderStage::Vertex, "", "", vertex_guid, env, false});
    inputs.push_back({get_filename(), "PSMain", shader::ShaderStage::Pixel, "", "", pixel_guid, env, false});

    app::get().get_shader_cache().compiler.add_multiple(inputs);
  }
}

implement_vertex_type(StaticMeshVertexType, "/Shaders/StaticMeshVertexType.hlsl");
implement_vertex_type(SkinnedMeshVertexType, "/Shaders/SkinnedMeshVertexType.hlsl");

implement_render_pass(ShadowMapRenderPass, "/Shaders/ShadowMapPass.hlsl");
implement_render_pass(GBufferRenderPass, "/Shaders/GBufferPass.hlsl");

implement_asset_factory(MaterialShaderFactory);
implement_asset_type(MaterialShader);
