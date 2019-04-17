#include <renderer/renderer.h>
#include <renderer/resource.h>
#include <graphicsinterface/graphicsinterface.h>
#include <app/app.h>
#include <runtime/static_mesh.h>
#include <runtime/static_mesh_resource.h>
#include <config/config.h>
#include <utils/path.h>
#include <shader/shader.h>
#include <shader/cache.h>

#include <game/static_mesh_component.h>
#include <game/transform_component.h>
#include <game/metadata_component.h>
#include <game/camera_component.h>
#include <game/light_component.h>

#include <input/input.h>
#include <window/window.h>

namespace gi = graphicsinterface;
// using component::ComponentRef;
using game::CameraComponent;
using game::LightComponent;
using game::StaticMeshComponent;
using game::TransformComponent;

using runtime::StaticMesh;
using runtime::StaticMeshRenderData;
using runtime::StaticMeshResource;
using shader::Shader;

using namespace glm;
namespace gi = graphicsinterface;

namespace renderer {
Shader *vertex;
Shader *pixel;

struct RenderTargetResource {
  void initialize(i32 width, i32 height, gi::PixelFormat pixel_format) {
    texture2d = gi::create_texture2d(width, height, pixel_format);
    render_target_view = gi::create_render_target_view(texture2d);
    srv = gi::create_shader_resource_view(texture2d);
  }
  gi::RenderTargetViewRef render_target_view;
  gi::Texture2DRef texture2d;
  gi::ShaderResourceViewRef srv;
};

struct GBuffer {
  RenderTargetResource world_pos;
  RenderTargetResource material_attributes;
  RenderTargetResource color;
  RenderTargetResource world_normal;

  gi::SamplerStateRef sampler_state;
  gi::RenderTargetViewRef rts[4];
};

struct Quad {
  gi::VertexBufferRef vertices;
  gi::VertexStream stream;
};

GBuffer gbuffer;
Quad quad;
shader::Shader *quad_vertex;
shader::Shader *quad_pixel;

shader::Shader *light_pass_ps;

RenderTargetResource color_intermediate_buffer;

void Renderer::load_shaders() {
  // create shader
  quad_vertex = &shader::load("/Shaders/quad.hlsl", gi::ShaderStage::Vertex, "VS");
  quad_pixel = &shader::load("/Shaders/quad.hlsl", gi::ShaderStage::Pixel, "PS");
  
  light_pass_ps = &shader::load("/Shaders/LightPass.hlsl", gi::ShaderStage::Pixel, "PS");

  vertex = &shader::load("/Shaders/main.hlsl", graphicsinterface::ShaderStage::Vertex, "VS");
  pixel = &shader::load("/Shaders/main.hlsl", graphicsinterface::ShaderStage::Pixel, "PS");
}

void Renderer::init_gbuffer() {
  auto &main_rtv = gi::get_main_render_target_view();
  i32 width, height;
  app::get().get_window().get_dimensions(width, height);
  // create 4 float rgba render targets
  auto pixel_format = gi::PixelFormat::FloatRGBA;
  gbuffer.sampler_state = gi::create_sampler_state();
  gbuffer.world_pos.initialize(width, height, pixel_format);
  gbuffer.material_attributes.initialize(width, height, pixel_format);
  gbuffer.color.initialize(width, height, pixel_format);
  gbuffer.world_normal.initialize(width, height, pixel_format);

  color_intermediate_buffer.initialize(width, height, pixel_format);

  gbuffer.rts[0] = gbuffer.world_pos.render_target_view;
  gbuffer.rts[1] = gbuffer.material_attributes.render_target_view;
  gbuffer.rts[2] = gbuffer.color.render_target_view;
  gbuffer.rts[3] = gbuffer.world_normal.render_target_view;
  // init quad here
  {
    float quadVertices[] = {
        // positions        // texture Coords
        -1.0f, 1.0f,  0.0f, 0.0f, 0.0f, //
        1.0f,  1.0f,  0.0f, 1.0f, 0.0f, //
        -1.0f, -1.0f, 0.0f, 0.0f, 1.0f, //
        1.0f,  -1.0f, 0.0f, 1.0f, 1.0f,
    };
    quad.vertices = gi::create_vertex_buffer(4, 20, gi::PixelFormat::Unknown, nullptr);
    gi::set_vertex_buffer_data(quadVertices, 80, quad.vertices);

    quad.stream.add({0, gi::SemanticName::Position, gi::PixelFormat::R32G32B32F, 0, 0});
    quad.stream.add({0, gi::SemanticName::TexCoord, gi::PixelFormat::R32G32F, 12, 0});
    quad.stream.add_buffer(quad.vertices.get());
  }
}

void Renderer::draw_quad() {
  gi::clear_render_target_view(gi::get_main_render_target_view(), glm::vec4(0));
  gi::clear_depth_stencil_view(gi::get_main_depth_stencil_view(), GI_CLEAR_DEPTH | GI_CLEAR_STENCIL, 1.f, 0.f);

  auto main_rt = gi::get_main_render_target_view();
  // bind these render targets
  gi::set_render_targets(1, &main_rt, gi::get_main_depth_stencil_view());
  // clear them
  gi::PipelineState state;
  state.bound_shaders.vertex = quad_vertex->compiled;
  state.bound_shaders.pixel = quad_pixel->compiled;
  state.primitive_type = gi::PrimitiveTopology::TriangleStrip;

  gi::ShaderParameter srv_parameter;
  //gi::set_shader_resource_view(srv_parameter, color_intermediate_buffer.srv, quad_pixel->compiled);
  gi::set_shader_resource_view(srv_parameter, gbuffer.color.srv, quad_pixel->compiled);
  gi::set_sampler_state(srv_parameter, gbuffer.sampler_state, quad_pixel->compiled);
  gi::set_pipeline_state(state);
  gi::set_vertex_stream(quad.stream, quad_vertex->compiled);
  gi::draw((u32)quad.vertices->count, 0);
}

void Renderer::draw_light_pass() {
  gi::DebugState gbuffer_debug_state("Light pass start");
  gi::set_render_targets(1, &color_intermediate_buffer.render_target_view, gi::get_main_depth_stencil_view());

  constexpr usize id = utils::string::hash_code("LightPassParams");
  auto uniform_buf = app::get().get_shader_cache().uniform_buffer_map[id];

  gi::PipelineState state;
  state.bound_shaders.vertex = quad_vertex->compiled;
  state.bound_shaders.pixel = light_pass_ps->compiled;
  state.primitive_type = gi::PrimitiveTopology::TriangleStrip;
  gi::set_vertex_stream(quad.stream, quad_vertex->compiled);
  gi::bind_uniform_buffer(0, gi::ShaderStage::Vertex, uniform_buf->get_resource());

  gi::set_shader_resource_view({0}, gbuffer.world_pos.srv, light_pass_ps->compiled);
  gi::set_shader_resource_view({1}, gbuffer.material_attributes.srv, light_pass_ps->compiled);
  gi::set_shader_resource_view({2}, gbuffer.color.srv, light_pass_ps->compiled);
  gi::set_shader_resource_view({3}, gbuffer.world_normal.srv, light_pass_ps->compiled);
  gi::set_sampler_state({0}, gbuffer.sampler_state, quad_pixel->compiled);
  gi::set_pipeline_state(state);

  
  auto &cameras = component::get_vector_of_components<CameraComponent>();
  assert(cameras.slots.size());
  auto camera_entity_id = component::get_entity_id<CameraComponent>(0);
  auto &xf = *component::find_and_get<TransformComponent>(camera_entity_id);
  uniform_buf->set_raw_parameter("viewPos", (void *)&xf.position[0], sizeof(vec3));

  auto &lights = component::get_array_of_components<LightComponent>();
  for (int linear_index = 0; linear_index < lights.size(); linear_index++) {
    auto &light = lights[linear_index];
    auto entity_id = component::get_entity_id<LightComponent>(linear_index);
    auto &transform = *component::find_and_get<TransformComponent>(entity_id);

    uniform_buf->set_raw_parameter("ligthPosition", (void *)&transform.position[0], sizeof(vec3));
    uniform_buf->set_raw_parameter("lightColor", (void *)&light.diffuse_alpha[0], sizeof(vec3));
    vec4 light_params = vec4(0.7, 1.8, 2.f, 1.f);
    uniform_buf->set_raw_parameter("lightParameters", (void *)&light_params[0], sizeof(vec4));
    uniform_buf->update_resource();

    gi::draw((u32)quad.vertices->count, 0);
  }
}

void Renderer::initialize() {
  using namespace utils::path;
  load_shaders();
  init_gbuffer();
}

void Renderer::draw_gbuffer() {
  gi::DebugState gbuffer_debug_state("Gbuffer start");

  gi::set_render_targets(4, gbuffer.rts, gi::get_main_depth_stencil_view());
  gi::clear_render_target_view(gbuffer.color.render_target_view, glm::vec4(0));

  auto &cameras = component::get_vector_of_components<CameraComponent>();
  assert(cameras.slots.size());
  auto &camera = *component::find_and_get<CameraComponent>(cameras.get_slot_id(0));

  mat4 projection = camera.projection_matrix;
  mat4 view = camera.view_matrix;
  view = transpose(view);
  projection = transpose(projection);

  // get components of type
  gi::PipelineState state;
  state.bound_shaders.pixel = pixel->compiled;
  state.bound_shaders.vertex = vertex->compiled;
  state.primitive_type = gi::PrimitiveTopology::TriangleList;
  gi::set_pipeline_state(state);

  constexpr usize camera_parameters_id = utils::string::hash_code("CameraParameters");
  constexpr usize model_parameters_id = utils::string::hash_code("ModelParameters");

  auto camera_uniform_buf = app::get().get_shader_cache().uniform_buffer_map[camera_parameters_id];
  auto model_uniform_buf = app::get().get_shader_cache().uniform_buffer_map[model_parameters_id];

  auto &static_meshes = component::get_array_of_components<StaticMeshComponent>();
  for (int x = 0; x < static_meshes.size(); x++) {
    auto &mesh = static_meshes[x];

    auto static_mesh_asset = mesh.static_mesh.get();
    if (!static_mesh_asset)
      continue;

    auto entity_id = component::get_entity_id<StaticMeshComponent>(x);
    auto meta = component::find_and_get<game::MetadataComponent>(entity_id);
#if 0
    String debug_str = utils::string::sprintf("Started drawing mesh : %s", meta->friendly_name.c_str());
    gi::DebugState state(debug_str.c_str());
#else
    gi::DebugState state("Draw mesh");
#endif

    mat4 model(1);
    mat4 model_previous(1);
    {
      auto transform = component::get_mut(mesh.cached_transform_component);
      if (!transform) {
        // cache transform
        mesh.cached_transform_component = component::find<TransformComponent>(entity_id);
        transform = component::get_mut(mesh.cached_transform_component);
      }

      if (transform) {
        StaticMesh *static_mesh_asset = mesh.static_mesh.get();
        auto &transform_cast = *transform;

        model = transpose(transform_cast.transform_matrix);
        model_previous = transpose(transform_cast.transform_matrix_previous);
      }
    }

    StaticMeshResource &resource = static_mesh_asset->get_render_resource();
    {
      // TODO move out of for loop...
      camera_uniform_buf->set_raw_parameter("viewMatrix", &view[0], sizeof(mat4));
      camera_uniform_buf->set_raw_parameter("projectionMatrix", &projection[0], sizeof(mat4));
      camera_uniform_buf->update_resource();
      gi::bind_uniform_buffer(0, gi::ShaderStage::Vertex, camera_uniform_buf->get_resource());

      model_uniform_buf->set_raw_parameter("modelMatrix", &model[0], sizeof(mat4));
      model_uniform_buf->set_raw_parameter("modelMatrixPrev", &model_previous[0], sizeof(mat4));
      model_uniform_buf->update_resource();
      gi::bind_uniform_buffer(1, gi::ShaderStage::Vertex, model_uniform_buf->get_resource());

      StaticMeshRenderData &render_data = resource.lod_resources[0];

      gi::set_vertex_stream(resource.vertex_stream, vertex->compiled);
      gi::draw_indexed(render_data.indices, gi::PrimitiveTopology::TriangleList, (u32)render_data.indices->get_count(),
                       0, 0);
    }
  }
}

void Renderer::draw_image() {
  // set custom render target
  draw_gbuffer();
  //draw_light_pass();
  draw_quad();

  gi::present();
}
} // namespace renderer
