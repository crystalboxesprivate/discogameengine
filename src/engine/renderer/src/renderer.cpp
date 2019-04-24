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
#include <runtime/environment_map.h>
#include <runtime/environment_map_resource.h>

#include <renderer/procedural_geo.h>
#include <runtime/texture2d_resource.h>
#include <runtime/texture2d.h>

#include <game/static_mesh_component.h>
#include <game/transform_component.h>
#include <game/metadata_component.h>
#include <game/material_component.h>
#include <game/camera_component.h>
#include <game/light_component.h>

#include <input/input.h>
#include <window/window.h>

#include <renderer/gbuffer.h>

namespace gi = graphicsinterface;
using game::CameraComponent;
using game::LightComponent;
using game::StaticMeshComponent;
using game::TransformComponent;

using runtime::EnvironmentMap;
using runtime::StaticMesh;
using runtime::Texture2D;
using shader::Shader;

using namespace glm;
namespace gi = graphicsinterface;

#define CONTENT_DIR(x) utils::path::join(config::CONTENT_DIR, x)

static const char *TEX2D_IBL_PATH = "iblarchive/panorama_map.iblarchive";
static const char *TEX2D_BRDF_LUT_PATH = "brdf_lut.hdr";
static const char *SHADER_QUAD_PATH = "/Shaders/quad.hlsl";
static const char *SHADER_CUBEMAP_PATH = "/Shaders/Skybox.hlsl";
static const char *SHADER_LIGHT_PATH = "/Shaders/LightPass.hlsl";
static const char *SHADER_IBL_PATH = "/Shaders/IBLPass.hlsl";

#define MAKE_MOTION_VECTOR_PASS 1

namespace renderer {
Quad *screen_quad = nullptr;

GBuffer gbuffer;
shader::Shader *light_pass_ps;

RenderTargetResource *color_output;
RenderTargetResource color_intermediate_buffer_a;
RenderTargetResource color_intermediate_buffer_b;

gi::BlendStateRef blend_state;
gi::BlendStateRef blend_state_skybox;

MaterialShader *material_shader;

struct SkyBox {
  EnvironmentMap *defaultTextureCube = nullptr;
  asset::AssetHandle<EnvironmentMap> asset;

  gi::VertexStream stream;
  gi::IndexBufferRef index_buffer;
  gi::VertexBufferRef vertex_buffer;

  Shader *vertex_shader;
  Shader *pixel_shader;

  void initialize() {
    // String cubepath = utils::path::join(config::CONTENT_DIR, "iblarchive/Sidewalk_At_Night.iblarchive");
    String cubepath = CONTENT_DIR(TEX2D_IBL_PATH);
    asset = asset::add<EnvironmentMap>(cubepath);

    defaultTextureCube = asset.get();
    Vector<glm::vec3> positions_arr;
    Vector<i32> indices_arr;

    sphere_geometry(indices_arr, positions_arr, 512.f, 10, 10);

    index_buffer = gi::create_index_buffer(indices_arr.size(), nullptr);
    gi::set_index_buffer_data(indices_arr.data(), indices_arr.size() * 4, index_buffer);

    vertex_buffer = gi::create_vertex_buffer(positions_arr.size(), 3 * 4, gi::PixelFormat::R32G32B32F, nullptr);
    gi::set_vertex_buffer_data(positions_arr.data(), positions_arr.size() * 3 * 4, vertex_buffer);

    stream.add({0, gi::SemanticName::Position, gi::PixelFormat::R32G32B32F, 0, 0});
    stream.add_buffer(vertex_buffer.get());
  }
};

SkyBox skybox;

struct PBS {
  Shader *ibl_pass;
  Texture2D *brdf_lut = nullptr;
};

PBS pbs;
shader::UniformBufferDescription *camera_uniform_buf = nullptr;

void Renderer::initialize() {
  using namespace utils::path;
  String brdf_path = CONTENT_DIR(TEX2D_BRDF_LUT_PATH);
  pbs.brdf_lut = asset::add<Texture2D>(brdf_path).get();
  asset::load_to_ram(*pbs.brdf_lut, false, true);

  load_shaders();
  gbuffer.initialize();

  {
    auto pixel_format = gi::PixelFormat::FloatRGBA;
    i32 width, height;
    app::get().get_window().get_dimensions(width, height);
    color_intermediate_buffer_a.initialize(width, height, pixel_format);
    color_intermediate_buffer_b.initialize(width, height, pixel_format);
  }

  quad.initialize();
  screen_quad = &quad;
  // init quad here

  blend_state = gi::addBlendState(gi::ONE, gi::ONE);
  blend_state_skybox = gi::addBlendState(gi::SRC_ALPHA, gi::ONE_MINUS_SRC_ALPHA);
  // Cubemap
  skybox.initialize();

  {
    // do post effect initialization here.
    post_effects.push_back(PostEffectRef(new MotionBlurEffect));
    post_effects.push_back(PostEffectRef(new TonemapEffect));

    for (auto &effect : post_effects) {
      effect->init();
    }
  }
}

void load_camera_params() {
  camera_uniform_buf = app::get().get_shader_cache().uniform_buffer_map[utils::string::hash_code("CameraParameters")];

  mat4 projection, view, view_previous;
  auto &cameras = component::get_vector_of_components<CameraComponent>();
  assert(cameras.slots.size());
  auto &camera = *component::find_and_get<CameraComponent>(cameras.get_slot_id(0));

  projection = transpose(camera.projection_matrix);
  view = transpose(camera.view_matrix);
  view_previous = transpose(camera.view_matrix_previous);

  auto &time = app::get().time;
  vec4 time_vec(time.current, time.delta_seconds, time.current * 3, time.current / 20.f);

  camera_uniform_buf->set_raw_parameter("viewMatrix", &view[0], sizeof(mat4));
  camera_uniform_buf->set_raw_parameter("_timeConstant", &time_vec[0], sizeof(vec4));
  camera_uniform_buf->set_raw_parameter("viewMatrixPrevious", &view_previous[0], sizeof(mat4));
  camera_uniform_buf->set_raw_parameter("projectionMatrix", &projection[0], sizeof(mat4));
  camera_uniform_buf->update_resource();
}

void Renderer::load_shaders() {
  // create shader
  quad.vertex = &shader::load(SHADER_QUAD_PATH, gi::ShaderStage::Vertex, "VS");
  quad.pixel = &shader::load(SHADER_QUAD_PATH, gi::ShaderStage::Pixel, "PS");

  light_pass_ps = &shader::load(SHADER_LIGHT_PATH, gi::ShaderStage::Pixel, "PS");

  skybox.vertex_shader = &shader::load(SHADER_CUBEMAP_PATH, gi::ShaderStage::Vertex, "VS");
  skybox.pixel_shader = &shader::load(SHADER_CUBEMAP_PATH, gi::ShaderStage::Pixel, "PS");

  auto gbuffer_vertex = &shader::load("/Shaders/main.hlsl", gi::ShaderStage::Vertex, "VS");
  auto gbuffer_pixel = &shader::load("/Shaders/main.hlsl", gi::ShaderStage::Pixel, "PS");

  pbs.ibl_pass = &shader::load(SHADER_IBL_PATH, gi::ShaderStage::Pixel, "PS");
}

void Renderer::draw_skybox() {
  assert(skybox.defaultTextureCube);
  gi::DebugState gbuffer_debug_state("Sky box pass");

  auto res = skybox.defaultTextureCube->get_resource();
  if (!res) {
    return;
  }
  // clear them
  gi::PipelineState state;
  state.bound_shaders.vertex = skybox.vertex_shader->compiled;
  state.bound_shaders.pixel = skybox.pixel_shader->compiled;
  state.primitive_type = gi::PrimitiveTopology::TriangleStrip;

  gi::bind_uniform_buffer(0, gi::ShaderStage::Vertex, camera_uniform_buf->get_resource());

  gi::ShaderParameter srv_parameter;
  gi::set_shader_resource_view(srv_parameter, res->color_srv, skybox.pixel_shader->compiled);
  gi::set_sampler_state(srv_parameter, res->sampler_state, skybox.pixel_shader->compiled);
  gi::set_pipeline_state(state);
  gi::set_vertex_stream(skybox.stream, skybox.vertex_shader->compiled);
  gi::draw_indexed(skybox.index_buffer, gi::PrimitiveTopology::TriangleList, (u32)skybox.index_buffer->get_count(), 0,
                   0);

  gi::set_shader_resource_view(srv_parameter, nullptr, skybox.pixel_shader->compiled);
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
  gi::set_render_targets(1, &color_intermediate_buffer_a.render_target_view, nullptr);
  gi::clear_render_target_view(color_intermediate_buffer_a.render_target_view, glm::vec4(0));
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
  pipeline_state.bound_shaders.vertex = quad.vertex->compiled;
  pipeline_state.primitive_type = gi::PrimitiveTopology::TriangleStrip;
  gi::set_vertex_stream(quad.stream, quad.vertex->compiled);

  constexpr usize id = utils::string::hash_code("LightPassParams");
  shader::UniformBufferDescription *uniform_buf = app::get().get_shader_cache().uniform_buffer_map[id];
  if (uniform_buf) {
    uniform_buf->set_raw_parameter("viewPos", (void *)&view_position[0], sizeof(vec3));
    uniform_buf->update_resource();
    gi::bind_uniform_buffer(0, gi::ShaderStage::Pixel, uniform_buf->get_resource());
  }

  ///////////////////////////////////////////////////////////////////////////////////////////
  // IBL
  ///////////////////////////////////////////////////////////////////////////////////////////

  auto res = skybox.defaultTextureCube->get_resource();
  auto brdf_res = pbs.brdf_lut->get_resource();
  if (res && brdf_res) {
    gi::ShaderRef ibl = pbs.ibl_pass->compiled;
    // First pass needs to draw the opaque geometry,
    // the rest of the lights will be drawn additively
    gi::set_blend_state(blend_state_skybox);
    pipeline_state.bound_shaders.pixel = ibl;
    gi::set_pipeline_state(pipeline_state);
    set_gbuffer(true, ibl);
    gi::set_shader_resource_view({4}, brdf_res->srv, ibl);
    gi::set_shader_resource_view({5}, res->color_srv, ibl);
    gi::set_shader_resource_view({6}, res->irradiance_srv, ibl);
    gi::set_shader_resource_view({7}, res->prefilter_srv, ibl);
    gi::set_sampler_state({1}, res->sampler_state, ibl);

    gi::draw((u32)quad.vertices->count, 0);

    gi::set_shader_resource_view({4}, nullptr, ibl);
    gi::set_shader_resource_view({5}, nullptr, ibl);
    gi::set_shader_resource_view({6}, nullptr, ibl);
    gi::set_shader_resource_view({7}, nullptr, ibl);
    set_gbuffer(false, ibl);
  }
  ///////////////////////////////////////////////////////////////////////////////////////////
  gi::set_blend_state(nullptr);

#if 0
  // Draw lights
  pipeline_state.bound_shaders.pixel = light_pass_ps->compiled;
  set_gbuffer(true, light_pass_ps->compiled);

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
#endif
}

void Renderer::draw_quad() {
  auto main_rt = gi::get_main_render_target_view();
  // bind these render targets
  gi::set_render_targets(1, &main_rt, nullptr);
  gi::clear_render_target_view(main_rt, glm::vec4(0));

  // clear them
  gi::PipelineState state;
  state.bound_shaders.vertex = quad.vertex->compiled;
  state.bound_shaders.pixel = quad.pixel->compiled;
  state.primitive_type = gi::PrimitiveTopology::TriangleStrip;

  gi::ShaderParameter srv_parameter;
  // gi::set_shader_resource_view(srv_parameter, gbuffer.world_normal.srv, quad.pixel->compiled);
  // gi::set_shader_resource_view(srv_parameter, gbuffer.motion_vectors.srv, quad.pixel->compiled);
  gi::set_shader_resource_view(srv_parameter, color_output->srv, quad.pixel->compiled);
  gi::set_sampler_state(srv_parameter, gbuffer.sampler_state, quad.pixel->compiled);
  gi::set_pipeline_state(state);
  gi::set_vertex_stream(quad.stream, quad.vertex->compiled);
  gi::draw((u32)quad.vertices->count, 0);

  gi::set_shader_resource_view(srv_parameter, nullptr, quad.pixel->compiled);
}

void Renderer::draw_image() {
  gi::clear_render_target_view(gi::get_main_render_target_view(), glm::vec4(0));
  gi::clear_depth_stencil_view(gi::get_main_depth_stencil_view(), GI_CLEAR_DEPTH | GI_CLEAR_STENCIL, 1.f, 0.f);

  load_camera_params();

  // draw skybox
  gbuffer.draw_static_meshes();
  gbuffer.draw_skinned_meshes();


  gi::set_z_buffer(false);

  draw_light_pass();

  /*
    post effects go here...
  */
  {
    color_output = &color_intermediate_buffer_a;
    RenderTargetResource *render_target = &color_intermediate_buffer_b;

    for (auto &effect : post_effects) {
      effect->execute(color_output->srv, render_target->render_target_view, nullptr, &gbuffer);

      RenderTargetResource *temp = color_output;
      color_output = render_target;
      render_target = temp;
    }
  }

  draw_quad();

  gi::set_z_buffer(true);
  // ui and text should be here

  gi::present();
}
} // namespace renderer
