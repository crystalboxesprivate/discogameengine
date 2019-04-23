#include <renderer/renderer.h>
#include <renderer/resource.h>
#include <renderer/rendercore.h>
#include <graphicsinterface/graphicsinterface.h>
#include <app/app.h>
#include <config/config.h>
#include <utils/path.h>
#include <shader/shader.h>
#include <shader/cache.h>

#include <runtime/static_mesh.h>
#include <runtime/static_mesh_resource.h>
#include <runtime/texture2d_resource.h>
#include <runtime/texture2d.h>
#include <runtime/texturecube.h>
#include <runtime/texturecube_resource.h>

#include <renderer/procedural_geo.h>

#include <game/static_mesh_component.h>
#include <game/transform_component.h>
#include <game/metadata_component.h>
#include <game/material_component.h>
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
using runtime::TextureCube;
using shader::Shader;

using namespace glm;
namespace gi = graphicsinterface;

namespace renderer {
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
Quad quad;

GBuffer gbuffer;
shader::Shader *quad_vertex;
shader::Shader *quad_pixel;
shader::Shader *light_pass_ps;

Shader *gbuffer_vertex;
Shader *gbuffer_pixel;

RenderTargetResource color_intermediate_buffer;
gi::BlendStateRef blend_state;
gi::BlendStateRef blend_state_skybox;

MaterialShader *default_shader;

struct SkyBox {
  TextureCube *defaultTextureCube = nullptr;
  asset::AssetHandle<TextureCube> asset;

  gi::VertexStream stream;
  gi::IndexBufferRef indices;
  gi::VertexBufferRef vertices;

  Shader *vertex_shader;
  Shader *pixel_shader;
};
SkyBox test_skybox;

struct DefaultTexture {
  gi::Texture2DRef texture;
  gi::ShaderResourceViewRef srv;

  gi::Texture2DRef normal;
  gi::ShaderResourceViewRef srv_normal;

  gi::SamplerStateRef sampler;
};

struct PBS {
  Shader *ibl_pass;
};

PBS pbs;

DefaultTexture default_texture;
shader::UniformBufferDescription *camera_uniform_buf = nullptr;

void Renderer::create_default_texture() {
  u8 one_pixel[] = {255, 255, 255, 255};
  default_texture.texture = gi::create_texture2d(1, 1, gi::PixelFormat::R8G8B8A8F, one_pixel);
  default_texture.srv = gi::create_shader_resource_view(default_texture.texture);
  default_texture.sampler = gi::create_sampler_state();

  u8 one_pixel_normal[] = {128, 128, 255, 255};
  default_texture.normal = gi::create_texture2d(1, 1, gi::PixelFormat::R8G8B8A8F, one_pixel_normal);
  default_texture.srv_normal = gi::create_shader_resource_view(default_texture.normal);
}

void Renderer::create_skybox() {
  String cubepath = utils::path::join(
      config::CONTENT_DIR,
      "textures/ExportedCubemaps/road_in_tenerife_mountain/road_in_tenerife_mountain.hdr.cubemapjson");
  test_skybox.asset = asset::add<TextureCube>(cubepath);
  test_skybox.defaultTextureCube = test_skybox.asset.get();
  {
    Vector<glm::vec3> positions;
    Vector<i32> indices;

    sphere_geometry(indices, positions, 512.f, 32, 32);

    test_skybox.indices = gi::create_index_buffer(indices.size(), nullptr);
    gi::set_index_buffer_data(indices.data(), indices.size() * 4, test_skybox.indices);

    test_skybox.vertices = gi::create_vertex_buffer(positions.size(), 3 * 4, gi::PixelFormat::R32G32B32F, nullptr);
    gi::set_vertex_buffer_data(positions.data(), positions.size() * 3 * 4, test_skybox.vertices);

    test_skybox.stream.add({0, gi::SemanticName::Position, gi::PixelFormat::R32G32B32F, 0, 0});
    test_skybox.stream.add_buffer(test_skybox.vertices.get());
  }
}

void Renderer::initialize() {
  using namespace utils::path;
  load_shaders();
  init_gbuffer();
  // Cubemap
  create_skybox();
  create_default_texture();
}

void load_camera_params() {
  camera_uniform_buf = app::get().get_shader_cache().uniform_buffer_map[utils::string::hash_code("CameraParameters")];

  mat4 projection, view;
  auto &cameras = component::get_vector_of_components<CameraComponent>();
  assert(cameras.slots.size());
  auto &camera = *component::find_and_get<CameraComponent>(cameras.get_slot_id(0));

  projection = transpose(camera.projection_matrix);
  view = transpose(camera.view_matrix);

  camera_uniform_buf->set_raw_parameter("viewMatrix", &view[0], sizeof(mat4));
  camera_uniform_buf->set_raw_parameter("projectionMatrix", &projection[0], sizeof(mat4));
  camera_uniform_buf->update_resource();
}

void get_transform(u32 entity_id, component::ComponentHandle2<TransformComponent> &handle,
                   shader::UniformBufferDescription *model_uniform_buf) {
  auto transform = component::get_mut(handle);
  if (!transform) {
    // cache transform
    handle = component::find<TransformComponent>(entity_id);
    transform = component::get_mut(handle);
  }
  assert(transform);
  auto &transform_cast = *transform;

  mat4 model = transpose(transform_cast.transform_matrix);
  mat4 model_previous = transpose(transform_cast.transform_matrix_previous);

  model_uniform_buf->set_raw_parameter("modelMatrix", &model[0], sizeof(mat4));
  model_uniform_buf->set_raw_parameter("modelMatrixPrev", &model_previous[0], sizeof(mat4));
  model_uniform_buf->update_resource();
}

void Renderer::load_shaders() {
  // create shader
  quad_vertex = &shader::load("/Shaders/quad.hlsl", gi::ShaderStage::Vertex, "VS");
  quad_pixel = &shader::load("/Shaders/quad.hlsl", gi::ShaderStage::Pixel, "PS");

  light_pass_ps = &shader::load("/Shaders/LightPass.hlsl", gi::ShaderStage::Pixel, "PS");

  test_skybox.vertex_shader = &shader::load("/Shaders/Skybox.hlsl", graphicsinterface::ShaderStage::Vertex, "VS");
  test_skybox.pixel_shader = &shader::load("/Shaders/Skybox.hlsl", graphicsinterface::ShaderStage::Pixel, "PS");

  gbuffer_vertex = &shader::load("/Shaders/main.hlsl", graphicsinterface::ShaderStage::Vertex, "VS");
  gbuffer_pixel = &shader::load("/Shaders/main.hlsl", graphicsinterface::ShaderStage::Pixel, "PS");

  pbs.ibl_pass = &shader::load("/Shaders/IBLPass.hlsl", gi::ShaderStage::Pixel, "PS");
}

void Renderer::init_gbuffer() {
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

  blend_state = gi::addBlendState(gi::ONE, gi::ONE);
  blend_state_skybox = gi::addBlendState(gi::SRC_ALPHA, gi::ONE_MINUS_SRC_ALPHA);
}

void Renderer::draw_skybox() {
  assert(test_skybox.defaultTextureCube);
  gi::DebugState gbuffer_debug_state("Sky box pass");

  auto res = test_skybox.defaultTextureCube->get_resource();
  if (!res) {
    return;
  }
  // clear them
  gi::PipelineState state;
  state.bound_shaders.vertex = test_skybox.vertex_shader->compiled;
  state.bound_shaders.pixel = test_skybox.pixel_shader->compiled;
  state.primitive_type = gi::PrimitiveTopology::TriangleStrip;

  gi::bind_uniform_buffer(0, gi::ShaderStage::Vertex, camera_uniform_buf->get_resource());

  gi::ShaderParameter srv_parameter;
  gi::set_shader_resource_view(srv_parameter, res->srv_color, test_skybox.pixel_shader->compiled);
  gi::set_sampler_state(srv_parameter, res->sampler_state, test_skybox.pixel_shader->compiled);
  gi::set_pipeline_state(state);
  gi::set_vertex_stream(test_skybox.stream, test_skybox.vertex_shader->compiled);
  gi::draw_indexed(test_skybox.indices, gi::PrimitiveTopology::TriangleList, (u32)test_skybox.indices->get_count(), 0,
                   0);

  gi::set_shader_resource_view(srv_parameter, nullptr, test_skybox.pixel_shader->compiled);
}

void set_gbuffer(bool state, gi::ShaderRef pixel_shader) {
  gi::set_sampler_state({0}, gbuffer.sampler_state, pixel_shader);
  gi::set_shader_resource_view({0}, state ? gbuffer.world_pos.srv : nullptr, pixel_shader);
  gi::set_shader_resource_view({1}, state ? gbuffer.material_attributes.srv : nullptr, pixel_shader);
  gi::set_shader_resource_view({2}, state ? gbuffer.color.srv : nullptr, pixel_shader);
  gi::set_shader_resource_view({3}, state ? gbuffer.world_normal.srv : nullptr, pixel_shader);
}

void Renderer::draw_light_pass() {
  gi::DebugState gbuffer_debug_state("Light pass start");

  gi::set_render_targets(1, &color_intermediate_buffer.render_target_view, nullptr);
  gi::clear_render_target_view(color_intermediate_buffer.render_target_view, glm::vec4(0));
  draw_skybox();

  vec3 view_position, view_direction;
  {
    auto &cameras = component::get_vector_of_components<CameraComponent>();
    assert(cameras.slots.size());
    u32 camera_entity_id = component::get_entity_id<CameraComponent>(0);
    const game::TransformComponent &transform_component =
        *component::find_and_get<TransformComponent>(camera_entity_id);
    view_position = transform_component.position;
    view_direction = vec3(0, 0, 1) * transform_component.rotation;
  }

  gi::PipelineState pipeline_state;
  pipeline_state.bound_shaders.vertex = quad_vertex->compiled;
  pipeline_state.primitive_type = gi::PrimitiveTopology::TriangleStrip;
  gi::set_vertex_stream(quad.stream, quad_vertex->compiled);

  ///////////////////////////////////////////////////////////////////////////////////////////
  // IBL
  ///////////////////////////////////////////////////////////////////////////////////////////

  auto res = test_skybox.defaultTextureCube->get_resource();
  if (res) {
    gi::ShaderRef ibl = pbs.ibl_pass->compiled;
    // First pass needs to draw the opaque geometry,
    // the rest of the lights will be drawn additively
    gi::set_blend_state(blend_state_skybox);
    pipeline_state.bound_shaders.pixel = ibl;
    gi::set_pipeline_state(pipeline_state);
    set_gbuffer(true, ibl);
    gi::set_shader_resource_view({4}, res->srv_irradiance, ibl);
    gi::set_shader_resource_view({5}, res->srv_color, ibl);

    gi::draw((u32)quad.vertices->count, 0);

    gi::set_shader_resource_view({4}, nullptr, ibl);
    gi::set_shader_resource_view({5}, nullptr, ibl);
    set_gbuffer(false, ibl);
  }
  ///////////////////////////////////////////////////////////////////////////////////////////
  gi::set_blend_state(nullptr);
  return;

  // Draw lights
  pipeline_state.bound_shaders.pixel = light_pass_ps->compiled;
  set_gbuffer(true, light_pass_ps->compiled);

  constexpr usize id = utils::string::hash_code("LightPassParams");
  shader::UniformBufferDescription *uniform_buf = app::get().get_shader_cache().uniform_buffer_map[id];

  if (uniform_buf) {
    uniform_buf->set_raw_parameter("viewPos", (void *)&view_position[0], sizeof(vec3));
    gi::bind_uniform_buffer(0, gi::ShaderStage::Pixel, uniform_buf->get_resource());
  }

  gi::set_pipeline_state(pipeline_state);
  gi::set_blend_state(blend_state);

  auto &lights = component::get_array_of_components<LightComponent>();
  for (int linear_index = 0; linear_index < lights.size(); linear_index++) {
    game::LightComponent &light = lights[linear_index];
    u32 entity_id = component::get_entity_id<LightComponent>(linear_index);
    const game::TransformComponent &transform = *component::find_and_get<TransformComponent>(entity_id);

    if (uniform_buf) {
      vec4 light_params = vec4(0.7, 1.8, 2000.f, 1.f);
      vec3 forward = vec3(0, 0, 1) * transform.rotation;
      uniform_buf->set_raw_parameter("lightDirection", (void *)&forward[0], sizeof(vec3));
      uniform_buf->set_raw_parameter("lightPosition", (void *)&transform.position[0], sizeof(vec3));
      uniform_buf->set_raw_parameter("lightColor", (void *)&light.diffuse_alpha[0], sizeof(vec3));
      uniform_buf->set_raw_parameter("lightParameters", (void *)&light_params[0], sizeof(vec4));
      uniform_buf->update_resource();
    }
    gi::draw((u32)quad.vertices->count, 0);
  }
  gi::set_blend_state(nullptr);
  set_gbuffer(false, light_pass_ps->compiled);
}

void Renderer::draw_quad() {
  auto main_rt = gi::get_main_render_target_view();
  // bind these render targets
  gi::set_render_targets(1, &main_rt, nullptr);
  gi::clear_render_target_view(main_rt, glm::vec4(0));

  // clear them
  gi::PipelineState state;
  state.bound_shaders.vertex = quad_vertex->compiled;
  state.bound_shaders.pixel = quad_pixel->compiled;
  state.primitive_type = gi::PrimitiveTopology::TriangleStrip;

  gi::ShaderParameter srv_parameter;
  // gi::set_shader_resource_view(srv_parameter, gbuffer.world_normal.srv, quad_pixel->compiled);
  gi::set_shader_resource_view(srv_parameter, color_intermediate_buffer.srv, quad_pixel->compiled);
  gi::set_sampler_state(srv_parameter, gbuffer.sampler_state, quad_pixel->compiled);
  gi::set_pipeline_state(state);
  gi::set_vertex_stream(quad.stream, quad_vertex->compiled);
  gi::draw((u32)quad.vertices->count, 0);

  gi::set_shader_resource_view(srv_parameter, nullptr, quad_pixel->compiled);
}

gi::ShaderResourceViewRef get_texture(asset::AssetHandle<runtime::Texture2D> &handle, bool normal = false) {
  runtime::Texture2D *texture_asset = handle.get();
  if (!texture_asset)
    return normal ? default_texture.srv_normal : default_texture.srv;
  runtime::Texture2DResource *resource = texture_asset->get_resource();
  if (!resource)
    return normal ? default_texture.srv_normal : default_texture.srv;
  if (!resource->srv)
    return normal ? default_texture.srv_normal : default_texture.srv;
  return resource->srv;
}

void Renderer::draw_static_mesh_gbuffer() {
  gi::DebugState gbuffer_debug_state("Gbuffer start");

  gi::set_render_targets(4, gbuffer.rts, gi::get_main_depth_stencil_view());
  gi::clear_render_target_view(gbuffer.world_pos.render_target_view, glm::vec4(0));
  gi::clear_render_target_view(gbuffer.material_attributes.render_target_view, glm::vec4(0));
  gi::clear_render_target_view(gbuffer.color.render_target_view, glm::vec4(0));
  gi::clear_render_target_view(gbuffer.world_normal.render_target_view, glm::vec4(0));

  // Get components of type.
  gi::PipelineState state;
#if 0
  state.bound_shaders.vertex = gbuffer_vertex->compiled;
  state.bound_shaders.pixel = gbuffer_pixel->compiled;
#else
  if (!default_shader)
    default_shader = app::get().get_shader_cache().default_shader;
  auto &g_buffer_hader_pair = default_shader->compiled_shaders[StaticMeshVertexType::guid()];
  {
    if (!g_buffer_hader_pair.vertex.instance) {
      g_buffer_hader_pair.vertex.instance = app::get().get_shader_cache().shaders[g_buffer_hader_pair.vertex.global_id];
    }
    if (!g_buffer_hader_pair.pixel.instance) {
      g_buffer_hader_pair.pixel.instance = app::get().get_shader_cache().shaders[g_buffer_hader_pair.pixel.global_id];
    }
  }
  state.bound_shaders.vertex = g_buffer_hader_pair.vertex.instance;
  state.bound_shaders.pixel = g_buffer_hader_pair.pixel.instance;
#endif

  state.primitive_type = gi::PrimitiveTopology::TriangleList;
  gi::set_pipeline_state(state);
  gi::bind_uniform_buffer(0, gi::ShaderStage::Vertex, camera_uniform_buf->get_resource());

  auto model_uniform_buf =
      app::get().get_shader_cache().uniform_buffer_map[utils::string::hash_code("ModelParameters")];

  gi::bind_uniform_buffer(1, gi::ShaderStage::Vertex, model_uniform_buf->get_resource());

  auto &static_meshes = component::get_array_of_components<StaticMeshComponent>();
  for (int x = 0; x < static_meshes.size(); x++) {
    auto &mesh = static_meshes[x];

    auto static_mesh_asset = mesh.static_mesh.get();
    if (!static_mesh_asset)
      continue;

    gi::DebugState draw_mesh_debug("Draw mesh");
    auto entity_id = component::get_entity_id<StaticMeshComponent>(x);
    get_transform(entity_id, mesh.cached_transform_component, model_uniform_buf);

    StaticMeshResource *resource_ptr = static_mesh_asset->get_render_resource();
    if (!resource_ptr) {
      continue;
    }
    StaticMeshResource &resource = *resource_ptr;
    StaticMeshRenderData &render_data = resource.lod_resources[0];
    {
      using game::MaterialComponent;
      MaterialComponent &material = *component::find_and_get_mut<MaterialComponent>(entity_id);
      gi::set_sampler_state({0}, default_texture.sampler, state.bound_shaders.pixel);
      gi::set_shader_resource_view({0}, get_texture(material.albedo_texture), state.bound_shaders.pixel);
      gi::set_shader_resource_view({1}, get_texture(material.mask_texture), state.bound_shaders.pixel);
      gi::set_shader_resource_view({2}, get_texture(material.normal_texture), state.bound_shaders.pixel);
    }
    gi::set_vertex_stream(resource.vertex_stream, gbuffer_vertex->compiled);
    gi::draw_indexed(render_data.indices, gi::PrimitiveTopology::TriangleList, (u32)render_data.indices->get_count(), 0,
                     0);
  }
}

void Renderer::draw_image() {
  gi::clear_render_target_view(gi::get_main_render_target_view(), glm::vec4(0));
  gi::clear_depth_stencil_view(gi::get_main_depth_stencil_view(), GI_CLEAR_DEPTH | GI_CLEAR_STENCIL, 1.f, 0.f);

  load_camera_params();

  // draw skybox

  draw_static_mesh_gbuffer();

  gi::set_z_buffer(false);
  // draw_skybox();

  draw_light_pass();

  draw_quad();

  /*
    post effects go here...
  */

  gi::set_z_buffer(true);
  // ui and text should be here

  gi::present();
}
} // namespace renderer
