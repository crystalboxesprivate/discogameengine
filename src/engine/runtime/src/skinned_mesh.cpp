#include <runtime/skinned_mesh.h>
#include <utils/string.h>

using utils::string::hash_code;
using namespace glm;

using namespace runtime;

namespace gi = graphicsinterface;
using gi::PixelFormat;

using runtime::animation::calculate_glm_interpolated_position;
using runtime::animation::calculate_glm_interpolated_rotation;
using runtime::animation::calculate_glm_interpolated_scaling;
using runtime::animation::find_position;
using runtime::animation::find_rotation;
using runtime::animation::find_scaling;

SkinnedMeshResource::SkinnedMeshResource() {
  namespace gi = graphicsinterface;
  vertex_stream.count = 0;
  vertex_stream.add({0, gi::SemanticName::Position, PixelFormat::FloatRGBA, 0u, 0u});
  vertex_stream.add({0, gi::SemanticName::Normal, PixelFormat::FloatRGBA, 16, 0u});
  vertex_stream.add({0, gi::SemanticName::TexCoord, PixelFormat::FloatRGBA, 32, 0u});

  vertex_stream.add({0, gi::SemanticName::Tangent, PixelFormat::FloatRGBA, 48, 0u});
  vertex_stream.add({0, gi::SemanticName::Binormal, PixelFormat::FloatRGBA, 64, 0u});

  vertex_stream.add({0, gi::SemanticName::TexCoord, PixelFormat::FloatRGBA, 80, 1});
  vertex_stream.add({0, gi::SemanticName::TexCoord, PixelFormat::FloatRGBA, 96, 2});
}

void SkinnedMesh::serialize(Archive &archive) {
  Asset::serialize(archive);
  archive << indices;
  archive << vertices;
  archive << bounds;

  archive << number_of_bones;
  archive << ticks_per_second;

  archive << animations;
  archive << global_inverse_transformation;

  archive << hierarchy;
}

SkinnedMeshResource *SkinnedMesh::get_render_resource() {
  // if it's not loaded then return default SkinnedMeshresource
  if (!render_data) {
    render_data = SharedPtr<SkinnedMeshResource>(new SkinnedMeshResource);
  }

  if (!render_data->index_buffer) {
    if (!is_loading) {
      asset::load_to_ram(*this, false, false);
    }

    if (!is_loaded_to_ram) {
      auto default_asset = asset::get_default<SkinnedMesh>(asset::Type::SkinnedMesh);
      if (default_asset)
        return default_asset->render_data.get();
      return nullptr;
    }

    render_data->index_buffer = gi::create_index_buffer(indices.size());
    usize index_byte_size = indices.size() * sizeof(i32);
    gi::set_index_buffer_data(indices.data(), index_byte_size, render_data->index_buffer);

    usize stride = sizeof(SkinnedMeshVertex);
    render_data->vertex_buffer = gi::create_vertex_buffer(vertices.size(), stride, PixelFormat::FloatRGBA);
    usize vertex_byte_size = vertices.size() * stride;
    gi::set_vertex_buffer_data(vertices.data(), vertex_byte_size, render_data->vertex_buffer);
    free();

    render_data->vertex_stream.add_buffer(render_data->vertex_buffer.get());
  }

  return render_data.get();
}

const runtime::animation::Channel *
find_node_animation_channel(const runtime::animation::Animation &in_animation, const String &boneName) {
  for (u32 channel_index = 0; channel_index != in_animation.channels.size(); channel_index++) {
    if (in_animation.channels[channel_index].node_name == boneName) {
      return &in_animation.channels[channel_index];
    }
  }
  return nullptr;
}

void /*SkinnedMesh::*/read_node_hierarchy(Vector<glm::mat4> &transforms, Vector<glm::mat4> &object_bone_transforms, const Vector<glm::mat4> &offsets,
                                      float animation_time, const runtime::animation::Animation &animation,
                                      const runtime::animation::Node &node, const mat4 &parent_transform_matrix, const mat4&global_inverse_transform) {
  // Transformation of the node in bind pose
  i32 bone_idx = node.bone_index;
  const auto &node_name = node.name;
  mat4 node_transform = node.transform;
  const runtime::animation::Channel *node_anim = find_node_animation_channel(animation, node_name);
  if (node_anim) {
    // Get interpolated scaling
    vec3 scale;
    calculate_glm_interpolated_scaling(animation_time, *node_anim, scale);
    mat4 scaling_m = glm::scale(mat4(1.0f), scale);

    // Get interpolated rotation (quaternion)
    quat ori;
    calculate_glm_interpolated_rotation(animation_time, *node_anim, ori);
    mat4 rotation_m = mat4_cast(ori);

    // Get interpolated position
    vec3 pos;
    calculate_glm_interpolated_position(animation_time, *node_anim, pos);
    mat4 translation_m = translate(mat4(1.0f), pos);

    // Combine the above transformations
    node_transform = translation_m * rotation_m * scaling_m;
  }

  mat4 object_bone_transform = parent_transform_matrix * node_transform;

  if (bone_idx != -1) {
    object_bone_transforms[bone_idx] = object_bone_transform;
    transforms[bone_idx] = global_inverse_transform * object_bone_transform * offsets[bone_idx];
   }

  for (u32 child_index = 0; child_index != node.children.size(); child_index++) {
    read_node_hierarchy(transforms, object_bone_transforms, offsets, animation_time, animation, node.children[child_index],
                              object_bone_transform, global_inverse_transform);
  }
}


void SkinnedMesh::bone_transform(const SkinnedMesh& skinned_mesh, float time_in_seconds, u64 animation_name, std::vector<mat4> &final_transformation,
                                 std::vector<mat4> &globals, std::vector<mat4> &offsets) {
  const runtime::animation::Animation *animation = skinned_mesh.get_animation(animation_name);
  
  if (!animation)
    return;

  float time_in_ticks = time_in_seconds * (float)skinned_mesh.ticks_per_second;
  float animation_time = fmod(time_in_ticks, animation->duration);
  mat4 ident(1.0f);

  final_transformation.resize(skinned_mesh.number_of_bones);
  globals.resize(skinned_mesh.number_of_bones);
  offsets = skinned_mesh.bone_offsets;


  read_node_hierarchy(final_transformation, globals, skinned_mesh.bone_offsets, animation_time, *animation, skinned_mesh.hierarchy, ident, skinned_mesh.global_inverse_transformation);

  for (u32 bone_index = 0; bone_index < skinned_mesh.number_of_bones; bone_index++) {
    final_transformation[bone_index] = transpose(final_transformation[bone_index]);
    globals[bone_index] = transpose(globals[bone_index]);
    offsets[bone_index] = transpose(offsets[bone_index]);
  }
}
