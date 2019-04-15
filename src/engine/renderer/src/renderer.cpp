#include <renderer/resource.h>
#include <graphicsinterface/graphicsinterface.h>
#include <app/app.h>
#include <runtime/static_mesh.h>
#include <runtime/static_mesh_resource.h>
#include <config/config.h>
#include <utils/path.h>
#include <shader/shader.h>
#include <shader/cache.h>
#include <renderer/renderer.h>

#include <game/static_mesh_component.h>
#include <game/transform_component.h>
#include <game/metadata_component.h>
#include <game/camera_component.h>

#include <input/input.h>
#include <window/window.h>

namespace gi = graphicsinterface;
// using component::ComponentRef;
using game::CameraComponent;
using game::StaticMeshComponent;
using game::TransformComponent;
using runtime::StaticMesh;
using runtime::StaticMeshRenderData;
using runtime::StaticMeshResource;
using shader::Shader;

using namespace glm;

namespace renderer {
Shader *vertex;
Shader *pixel;

void Renderer::initialize() {
  using namespace utils::path;
  vertex = &shader::load(join(join(config::CONTENT_DIR, "shaders"), "main.hlsl"),
                         graphicsinterface::ShaderStage::Vertex, "VS");
  pixel = &shader::load(join(join(config::CONTENT_DIR, "shaders"), "main.hlsl"), graphicsinterface::ShaderStage::Pixel,
                        "PS");
}
} // namespace renderer

namespace renderer {
void Renderer::draw_image() {

  auto &cameras = component::get_vector_of_components<CameraComponent>();
  assert(cameras.slots.size());
  auto &camera = *component::find_and_get<CameraComponent>(cameras.get_slot_id(0));

  mat4 projection = camera.projection_matrix;
  mat4 view = camera.view_matrix;
  view = transpose(view);
  projection = transpose(projection);

  gi::clear_render_target_view(gi::get_main_render_target_view(), input::keyboard::is_pressed(input::Key::Space)
                                                                      ? linear_color::red
                                                                      : linear_color::black);
  gi::clear_depth_stencil_view(gi::get_main_depth_stencil_view(), GI_CLEAR_DEPTH | GI_CLEAR_STENCIL, 1.f, 0.f);
  // get components of type
  gi::PipelineState state;
  state.bound_shaders.pixel = pixel->compiled;
  state.bound_shaders.vertex = vertex->compiled;
  state.primitive_type = gi::PrimitiveTopology::TriangleList;
  gi::set_pipeline_state(state);

  auto uniform_buffer = app::get().get_shader_cache().uniform_buffer_map["cbPerObject"];

  auto &static_meshes = component::get_array_of_components<StaticMeshComponent>();
  for (int x = 0; x < static_meshes.size(); x++) {
    auto &mesh = static_meshes[x];
    auto entity_id = component::get_entity_id<StaticMeshComponent>(x);
    auto meta = component::find_and_get<game::MetadataComponent>(entity_id);

    String debug_str = utils::string::sprintf("Started drawing mesh : %s", meta->friendly_name.c_str());

    gi::DebugState state(debug_str.c_str()); //"Started drawing mesh");

    auto transform = component::get_mut(mesh.cached_transform_component);//mesh.cached_transform_component.get_ref();
    if (!transform) {
      // cache transform
      mesh.cached_transform_component = component::find<TransformComponent>(entity_id);
      transform = component::get_mut(mesh.cached_transform_component);
    }

    mat4 model(1);

    if (transform) {
      StaticMesh *static_mesh_asset = mesh.static_mesh.get();
      auto &transform_cast = *transform;
      model = transform_cast.transform_matrix;
    }

    auto static_mesh_asset = mesh.static_mesh.get();
    if (!static_mesh_asset)
      continue;
    StaticMeshResource &resource = static_mesh_asset->get_render_resource();
    usize lod_index = 0;
    {
      // calculate matrix
      if (uniform_buffer) {
        model = transpose(model);

        uniform_buffer->set_raw_parameter("view", &view[0], sizeof(mat4));
        uniform_buffer->set_raw_parameter("model", &model[0], sizeof(mat4));
        uniform_buffer->set_raw_parameter("projection", &projection[0], sizeof(mat4));
        uniform_buffer->update_resource();
        auto uniform_buffer_resource = uniform_buffer->get_resource();
        gi::bind_uniform_buffer(0, gi::ShaderStage::Vertex, uniform_buffer_resource);
      }

      StaticMeshRenderData &render_data = resource.lod_resources[lod_index];

      gi::set_vertex_stream(resource.vertex_stream, vertex->compiled);

      u32 index_count = (u32)render_data.indices->get_count();
      u32 start_index = 0;
      u32 vertex_start_index = 0;
      gi::draw_indexed(render_data.indices, gi::PrimitiveTopology::TriangleList, index_count, start_index,
                       vertex_start_index);
    }
    //#endif
  }
  gi::present();
}
} // namespace renderer
