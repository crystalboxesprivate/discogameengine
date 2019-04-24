#include <renderer/gbuffer.h>
#include <app/app.h>
#include <window/window.h>

#include <renderer/rendercore.h>
#include <component/component.h>
#include <app/app.h>
#include <shader/cache.h>

#include <game/static_mesh_component.h>
#include <game/skinned_mesh_component.h>
#include <game/transform_component.h>
#include <runtime/static_mesh_resource.h>
#include <game/material_component.h>

#include <runtime/texture2d_resource.h>
#include <runtime/texture2d.h>
using namespace renderer;
using game::SkinnedMeshComponent;
using game::StaticMeshComponent;
using game::TransformComponent;
namespace gi = graphicsinterface;
using namespace glm;

using runtime::StaticMeshRenderData;
using runtime::StaticMeshResource;
using runtime::Texture2D;

namespace renderer {
extern MaterialShader *material_shader;
extern shader::UniformBufferDescription *camera_uniform_buf;

struct DefaultTexture {
  gi::Texture2DRef texture;
  gi::ShaderResourceViewRef srv;

  gi::Texture2DRef normal;
  gi::ShaderResourceViewRef srv_normal;

  gi::SamplerStateRef sampler;

  void initialize() {
    sampler = gi::create_sampler_state();
    u8 one_pixel[] = {255, 255, 255, 255};
    texture = gi::create_texture2d(1, 1, gi::PixelFormat::R8G8B8A8F, one_pixel);
    srv = gi::create_shader_resource_view(texture);

    u8 one_pixel_normal[] = {128, 128, 255, 255};
    normal = gi::create_texture2d(1, 1, gi::PixelFormat::R8G8B8A8F, one_pixel_normal);
    srv_normal = gi::create_shader_resource_view(normal);
  }
};
DefaultTexture default_texture;

} // namespace renderer

shader::UniformBufferDescription *model_uniform_buf = nullptr;

static const i32 MAXNUMBEROFBONES = 100;

struct MatrixBuffer {
  mat4x4 bones[MAXNUMBEROFBONES];
  mat4x4 bones_previous[MAXNUMBEROFBONES];
  // vec4 numBonesUsed;
};

gi::UniformBufferRef matrices;
void GBuffer::initialize() {
  i32 width, height;
  app::get().get_window().get_dimensions(width, height);
  // create 4 float rgba render targets
  auto pixel_format = gi::PixelFormat::FloatRGBA;
  sampler_state = gi::create_sampler_state();
  world_pos.initialize(width, height, pixel_format);
  material_attributes.initialize(width, height, pixel_format);
  color.initialize(width, height, pixel_format);
  world_normal.initialize(width, height, pixel_format);

  rts[0] = world_pos.render_target_view;
  rts[1] = material_attributes.render_target_view;
  rts[2] = color.render_target_view;
  rts[3] = world_normal.render_target_view;

#if MAKE_MOTION_VECTOR_PASS
  motion_vectors.initialize(width, height, pixel_format);
  rts[4] = motion_vectors.render_target_view;
#endif

  matrices = gi::create_uniform_buffer(sizeof(MatrixBuffer), nullptr);

  model_uniform_buf = app::get().get_shader_cache().uniform_buffer_map[utils::string::hash_code("ModelParameters")];
  default_texture.initialize();
}

void get_shaders(u32 vertex_type_id, gi::PipelineState::BoundShaders &bound_shaders) {
  if (!material_shader)
    material_shader = app::get().get_shader_cache().default_shader;
  auto &gbuf_shader_pair = material_shader->compiled_shaders[vertex_type_id];
  {
    if (!gbuf_shader_pair.vertex.instance) {
      gbuf_shader_pair.vertex.instance = app::get().get_shader_cache().shaders[gbuf_shader_pair.vertex.global_id];
    }
    if (!gbuf_shader_pair.pixel.instance) {
      gbuf_shader_pair.pixel.instance = app::get().get_shader_cache().shaders[gbuf_shader_pair.pixel.global_id];
    }
  }
  bound_shaders.vertex = gbuf_shader_pair.vertex.instance;
  bound_shaders.pixel = gbuf_shader_pair.pixel.instance;
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

void GBuffer::pass_start(graphicsinterface::PipelineState &state, u32 vertex_type) {

#if MAKE_MOTION_VECTOR_PASS
  gi::set_render_targets(5, rts, gi::get_main_depth_stencil_view());
#else
  gi::set_render_targets(4, rts, gi::get_main_depth_stencil_view());
#endif
  // Get components of type.
  get_shaders(vertex_type, state.bound_shaders);
  state.primitive_type = gi::PrimitiveTopology::TriangleList;
  gi::set_pipeline_state(state);

  gi::bind_uniform_buffer(0, gi::ShaderStage::Vertex, camera_uniform_buf->get_resource());
  gi::bind_uniform_buffer(1, gi::ShaderStage::Vertex, model_uniform_buf->get_resource());
}

void GBuffer::clear() {
  gi::clear_render_target_view(world_pos.render_target_view, glm::vec4(0));
  gi::clear_render_target_view(material_attributes.render_target_view, glm::vec4(0));
  gi::clear_render_target_view(color.render_target_view, glm::vec4(0));
  gi::clear_render_target_view(world_normal.render_target_view, glm::vec4(0));
#if MAKE_MOTION_VECTOR_PASS
  gi::clear_render_target_view(motion_vectors.render_target_view, glm::vec4(0));
#endif
}

void GBuffer::set_material_parameters(game::MaterialComponent *material_ptr, graphicsinterface::PipelineState &state) {
  if (!material_ptr) {
    // bind default textures
    gi::set_sampler_state({0}, default_texture.sampler, state.bound_shaders.pixel);
    gi::set_shader_resource_view({0}, default_texture.srv, state.bound_shaders.pixel);
    gi::set_shader_resource_view({1}, default_texture.srv, state.bound_shaders.pixel);
    gi::set_shader_resource_view({2}, default_texture.srv_normal, state.bound_shaders.pixel);
  } else {
    auto material = *material_ptr;
    gi::set_sampler_state({0}, default_texture.sampler, state.bound_shaders.pixel);
    gi::set_shader_resource_view({0}, get_texture(material.albedo_texture), state.bound_shaders.pixel);
    gi::set_shader_resource_view({1}, get_texture(material.mask_texture), state.bound_shaders.pixel);
    gi::set_shader_resource_view({2}, get_texture(material.normal_texture, true), state.bound_shaders.pixel);
  }
}
void GBuffer::draw_static_meshes() {
  gi::DebugState gbuffer_debug_state("Static mesh gbuffer start");
  gi::PipelineState state;
  pass_start(state, StaticMeshVertexType::guid());

  Vector<StaticMeshComponent> &static_meshes = component::get_array_of_components<StaticMeshComponent>();
  for (int x = 0; x < static_meshes.size(); x++) {
    StaticMeshComponent &static_mesh_component = static_meshes[x];
    runtime::StaticMesh *static_mesh_asset = static_mesh_component.mesh.get();
    if (!static_mesh_asset)
      continue;

    StaticMeshResource *resource_ptr = static_mesh_asset->get_render_resource();
    if (!resource_ptr) {
      continue;
    }

    gi::DebugState draw_mesh_debug("Draw static mesh");
    auto entity_id = component::get_entity_id<StaticMeshComponent>(x);
    get_transform(entity_id, static_mesh_component.cached_transform_component, model_uniform_buf);
    set_material_parameters(component::find_and_get_mut<game::MaterialComponent>(entity_id), state);

    StaticMeshResource &resource = *resource_ptr;
    gi::set_vertex_stream(resource.vertex_stream, state.bound_shaders.vertex);

    StaticMeshRenderData &render_data = resource.lod_resources[0];
    gi::draw_indexed(render_data.indices, gi::PrimitiveTopology::TriangleList, (u32)render_data.indices->get_count(), 0,
                     0);
  }
}

MatrixBuffer matrix_buffer;

void update_skinned_meshes() {
  Vector<SkinnedMeshComponent> &skinned_meshes = component::get_array_of_components<SkinnedMeshComponent>();
  for (int x = 0; x < skinned_meshes.size(); x++) {
    SkinnedMeshComponent &skinned_mesh_component = skinned_meshes[x];
    runtime::SkinnedMesh *skinned_mesh_asset = skinned_mesh_component.mesh.get();
    if (!skinned_mesh_asset)
      continue;

    auto &sm = *skinned_mesh_asset;
    {
      sm.state.active_animation.total_time = sm.get_duration_seconds(sm.state.active_animation.name);
      sm.state.active_animation.frame_step_time = (float)app::get().time.delta_seconds * 2.0;

      sm.state.active_animation.increment_time();

      Vector<glm::mat4x4> vec_final_transformation;
      Vector<glm::mat4x4> vec_offsets;

      sm.bone_transform(sm.state.active_animation.current_time, sm.state.active_animation.name,
                        skinned_mesh_component.bone_transforms, skinned_mesh_component.object_to_bone_transforms,
                        vec_offsets);

      skinned_mesh_component.number_of_bones_used = static_cast<uint32>(skinned_mesh_component.bone_transforms.size());

      memcpy(&matrix_buffer.bones_previous[0], &matrix_buffer.bones[0], sizeof(mat4x4) * MAXNUMBEROFBONES);
      memcpy(&matrix_buffer.bones[0], &skinned_mesh_component.bone_transforms[0],
             sizeof(mat4x4) * skinned_mesh_component.bone_transforms.size());

      gi::set_uniform_buffer_data(&matrix_buffer.bones[0], sizeof(MatrixBuffer), matrices);
    }
  }
}

void GBuffer::draw_skinned_meshes() {
  update_skinned_meshes();

  gi::DebugState gbuffer_debug_state("Skinned meshes");
  gi::PipelineState state;
  pass_start(state, SkinnedMeshVertexType::guid());

  Vector<SkinnedMeshComponent> &skinned_meshes = component::get_array_of_components<SkinnedMeshComponent>();
  for (int x = 0; x < skinned_meshes.size(); x++) {
    SkinnedMeshComponent &skinned_mesh_component = skinned_meshes[x];
    runtime::SkinnedMesh *skinned_mesh_asset = skinned_mesh_component.mesh.get();
    if (!skinned_mesh_asset)
      continue;

    runtime::SkinnedMeshResource *resource_ptr = skinned_mesh_asset->get_render_resource();
    if (!resource_ptr) {
      continue;
    }

    gi::DebugState draw_mesh_debug("Draw skinned mesh");
    auto entity_id = component::get_entity_id<SkinnedMeshComponent>(x);
    get_transform(entity_id, skinned_mesh_component.cached_transform_component, model_uniform_buf);
    set_material_parameters(component::find_and_get_mut<game::MaterialComponent>(entity_id), state);

    // set matrices
    {
      gi::bind_uniform_buffer(2, gi::ShaderStage::Vertex, matrices);
    }

    runtime::SkinnedMeshResource &resource = *resource_ptr;
    gi::set_vertex_stream(resource.vertex_stream, state.bound_shaders.vertex);

    gi::draw_indexed(resource.index_buffer, gi::PrimitiveTopology::TriangleList,
                     (u32)resource.index_buffer->get_count(), 0, 0);
  }
}
