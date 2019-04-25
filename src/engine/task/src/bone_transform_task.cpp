#include <task/bone_transform_task.h>
#include <game/skinned_mesh_component.h>
#include <game/animation_component.h>
#include <app/app.h>

using game::SkinnedMeshComponent;
using runtime::SkinnedMesh;

using namespace task;
using namespace glm;

void BoneTransformTask::init() {
}

const runtime::animation::Channel *find_node_animation_channel(const runtime::animation::Animation &in_animation,
                                                               const String &bone_name) {
  for (u32 channel_index = 0; channel_index != in_animation.channels.size(); channel_index++) {
    if (in_animation.channels[channel_index].node_name == bone_name) {
      return &in_animation.channels[channel_index];
    }
  }
  return nullptr;
}

void read_node_hierarchy(Vector<glm::mat4> &transforms, Vector<glm::mat4> &object_bone_transforms,
                         const Vector<glm::mat4> &offsets, float animation_time,
                         const runtime::animation::Animation &animation, const runtime::animation::Node &node,
                         const mat4 &parent_transform_matrix, const mat4 &global_inverse_transform) {
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
    read_node_hierarchy(transforms, object_bone_transforms, offsets, animation_time, animation,
                        node.children[child_index], object_bone_transform, global_inverse_transform);
  }
}

void bone_transform(const SkinnedMesh &skinned_mesh, float time_in_seconds, u64 animation_name,
                    Vector<glm::mat4> &final_transformation, Vector<glm::mat4> &globals, Vector<glm::mat4> &offsets) {

  const runtime::animation::Animation *animation = skinned_mesh.get_animation(animation_name);

  if (!animation)
    return;

  float time_in_ticks = time_in_seconds * (float)skinned_mesh.ticks_per_second;
  float animation_time = (float) fmod(time_in_ticks, animation->duration);
  mat4 ident(1.0f);

  final_transformation.resize(skinned_mesh.number_of_bones);
  globals.resize(skinned_mesh.number_of_bones);
  offsets = skinned_mesh.bone_offsets;

  read_node_hierarchy(final_transformation, globals, skinned_mesh.bone_offsets, animation_time, *animation,
                      skinned_mesh.hierarchy, ident, skinned_mesh.global_inverse_transformation);

  for (u32 bone_index = 0; bone_index < skinned_mesh.number_of_bones; bone_index++) {
    final_transformation[bone_index] = transpose(final_transformation[bone_index]);
    globals[bone_index] = transpose(globals[bone_index]);
    offsets[bone_index] = transpose(offsets[bone_index]);
  }
}

void BoneTransformTask::update() {
  Vector<SkinnedMeshComponent> &skinned_meshes = component::get_array_of_components<SkinnedMeshComponent>();
  for (int x = 0; x < skinned_meshes.size(); x++) {
    SkinnedMeshComponent &skinned_mesh_component = skinned_meshes[x];
    runtime::SkinnedMesh *skinned_mesh_asset = skinned_mesh_component.mesh.get();
    if (!skinned_mesh_asset)
      continue;

    // get entity id
    // get animation component
    auto entity_id = component::get_entity_id<SkinnedMeshComponent>(x);
    auto animation_ptr = component::find_and_get_mut<game::AnimationComponent>(entity_id);
    if (!animation_ptr)
      continue;
    auto &animation = *animation_ptr;

    auto &sm = *skinned_mesh_asset;
    {
      if (!sm.animations.size())
        continue;

      if (animation.active_animation.name == 0) {
        animation.active_animation.name = sm.default_animation;
        animation.default_animation.name = sm.default_animation;
      }

      u64 hash = animation.active_animation.name;
      auto anim = sm.get_animation(hash);
      if (!anim)
        continue;

      animation.active_animation.total_time = (float) anim->duration_seconds;
      animation.active_animation.frame_step_time = (float)app::get().time.delta_seconds * 2.0f;

      animation.active_animation.increment_time();

      Vector<mat4x4> vec_final_transformation;
      Vector<mat4x4> vec_offsets;

      bone_transform(sm, animation.active_animation.current_time, hash, skinned_mesh_component.bone_transforms,
                     skinned_mesh_component.object_to_bone_transforms, vec_offsets);

      skinned_mesh_component.number_of_bones_used = static_cast<uint32>(skinned_mesh_component.bone_transforms.size());

      memcpy(&skinned_mesh_component.matrix_buffer.bones_previous[0], &skinned_mesh_component.matrix_buffer.bones[0],
             sizeof(mat4x4) * SkinnedMeshComponent::MAX_NUMBER_OF_BONES);
      memcpy(&skinned_mesh_component.matrix_buffer.bones[0], &skinned_mesh_component.bone_transforms[0],
             sizeof(mat4x4) * skinned_mesh_component.bone_transforms.size());
    }
  }
}
