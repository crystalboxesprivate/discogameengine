#include <runtime/skinned_mesh.h>

using namespace runtime;

namespace gi = graphicsinterface;
using gi::PixelFormat;

SkinnedMeshResource::SkinnedMeshResource() {
  namespace gi = graphicsinterface;
  vertex_stream.count = 0;
  vertex_stream.add({0, gi::SemanticName::Position, gi::PixelFormat::FloatRGBA, 0u, 0u});
  vertex_stream.add({0, gi::SemanticName::Normal, gi::PixelFormat::FloatRGBA, 16, 0u});
  vertex_stream.add({0, gi::SemanticName::TexCoord, gi::PixelFormat::FloatRGBA, 32, 0u});

  vertex_stream.add({0, gi::SemanticName::Tangent, gi::PixelFormat::FloatRGBA, 48, 0u});
  vertex_stream.add({0, gi::SemanticName::Binormal, gi::PixelFormat::FloatRGBA, 64, 0u});

  vertex_stream.add({0, gi::SemanticName::TexCoord, gi::PixelFormat::FloatRGBA, 80, 1});
  vertex_stream.add({0, gi::SemanticName::TexCoord, gi::PixelFormat::FloatRGBA, 96, 2});
}

void SkinnedMesh::serialize(Archive &archive) {
  Asset::serialize(archive);
  archive << indices;
  archive << vertices;
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

float SkinnedMesh::get_duration_seconds(String animationName) {
  std::map<String, runtime::animation::Animation>::iterator itAnimation =
      this->animation_name_to_pscene.find(animationName);

  // Found it?
  if (itAnimation == this->animation_name_to_pscene.end()) { // Nope.
    return 0.0f;
  }

  // This is scaling the animation from 0 to 1
  return (float)itAnimation->second.duration / (float)itAnimation->second.ticks_per_second;
}

void SkinnedMesh::bone_transform(float time_in_seconds, String animation_name,
                                 std::vector<glm::mat4> &final_transformation, std::vector<glm::mat4> &globals,
                                 std::vector<glm::mat4> &offsets) {
  float time_in_ticks = time_in_seconds * (float) ticks_per_second;
  float animation_time = fmod(time_in_ticks, find_animation_total_time(animation_name));
  glm::mat4 ident(1.0f);
  this->read_node_hierarchy(animation_time, animation_name, hierarchy, ident);

  final_transformation.resize(this->number_of_bones);
  globals.resize(this->number_of_bones);
  offsets.resize(this->number_of_bones);

  for (u32 bone_index = 0; bone_index < this->number_of_bones; bone_index++) {
    final_transformation[bone_index] = glm::transpose(this->bone_info[bone_index].final_transformation);
    globals[bone_index] = this->bone_info[bone_index].object_bone_transformation;
    offsets[bone_index] = this->bone_info[bone_index].bone_offset;
  }
}
// Looks in the animation map and returns the total time
float SkinnedMesh::find_animation_total_time(String animation_name) {
  Map<String, runtime::animation::Animation>::iterator it_anim = this->animation_name_to_pscene.find(animation_name);

  // Found it?
  if (it_anim == this->animation_name_to_pscene.end()) { // Nope.
    return 0.0f;
  }
  return (float)it_anim->second.duration;
}

const runtime::animation::Channel *SkinnedMesh::find_node_animation_channel(runtime::animation::Animation *pAnimation,
                                                                            const String &boneName) {
  for (u32 ChannelIndex = 0; ChannelIndex != pAnimation->channels.size(); ChannelIndex++) {
    if (pAnimation->channels[ChannelIndex].node_name == boneName) {
      return &pAnimation->channels[ChannelIndex];
    }
  }
  return 0;
}

void SkinnedMesh::calculate_glm_interpolated_scaling(float animation_time, const runtime::animation::Channel *node_anim,
                                                     glm::vec3 &out) {
  if (node_anim->scaling_keys.size() == 1) {
    out.x = node_anim->scaling_keys[0].value.x;
    out.y = node_anim->scaling_keys[0].value.y;
    out.z = node_anim->scaling_keys[0].value.z;
    return;
  }

  u32 scaling_index = this->find_scaling(animation_time, node_anim);
  u32 next_scaling_index = (scaling_index + 1);
  assert(next_scaling_index < node_anim->scaling_keys.size());
  float delta_time =
      (float)(node_anim->scaling_keys[next_scaling_index].time - node_anim->scaling_keys[scaling_index].time);
  float factor = (animation_time - (float)node_anim->scaling_keys[scaling_index].time) / delta_time;

  if (factor < 0.0f)
    factor = 0.0f;
  if (factor > 1.0f)
    factor = 1.0f;

  const glm::vec3 &start = node_anim->scaling_keys[scaling_index].value;
  const glm::vec3 &end = node_anim->scaling_keys[next_scaling_index].value;

  out = (end - start) * factor + start;
  return;
}

void SkinnedMesh::read_node_hierarchy(float animation_time, String animation_name,
                                      const runtime::animation::Node &pNode, const glm::mat4 &ParentTransformMatrix) {

  const auto &node_name = pNode.name;

  // auto scene = (const aiScene *)this->pScene;
  runtime::animation::Animation *pAnimation = nullptr;
  Map<String, runtime::animation::Animation>::iterator itAnimation =
      this->animation_name_to_pscene.find(animation_name); // Animations

  // Did we find it?
  if (itAnimation != this->animation_name_to_pscene.end()) {
    pAnimation = &itAnimation->second;
  }

  // Transformation of the node in bind pose
  glm::mat4 node_transform = pNode.transform;
  const runtime::animation::Channel *node_anim = this->find_node_animation_channel(pAnimation, node_name);
  if (node_anim) {
    // Get interpolated scaling
    glm::vec3 scale;
    this->calculate_glm_interpolated_scaling(animation_time, node_anim, scale);
    glm::mat4 ScalingM = glm::scale(glm::mat4(1.0f), scale);

    // Get interpolated rotation (quaternion)
    glm::quat ori;
    this->calculate_glm_interpolated_rotation(animation_time, node_anim, ori);
    glm::mat4 RotationM = glm::mat4_cast(ori);

    // Get interpolated position
    glm::vec3 pos;
    this->calculate_glm_interpolated_position(animation_time, node_anim, pos);
    glm::mat4 translation_m = glm::translate(glm::mat4(1.0f), pos);

    // Combine the above transformations
    node_transform = translation_m * RotationM * ScalingM;
  }

  glm::mat4 object_bone_transform = ParentTransformMatrix * node_transform;

  std::map<String, u32>::iterator it = this->bone_name_to_bone_index.find(node_name);
  if (it != this->bone_name_to_bone_index.end()) {
    u32 BoneIndex = it->second;
    this->bone_info[BoneIndex].object_bone_transformation = object_bone_transform;
    this->bone_info[BoneIndex].final_transformation =
        this->global_inverse_transformation * object_bone_transform * this->bone_info[BoneIndex].bone_offset;

  } else {
    int breakpoint = 0;
  }

  for (u32 child_index = 0; child_index != pNode.children.size(); child_index++) {
    this->read_node_hierarchy(animation_time, animation_name, pNode.children[child_index], object_bone_transform);
  }
}

void SkinnedMesh::calculate_glm_interpolated_rotation(float animation_time,
                                                      const runtime::animation::Channel *node_anim, glm::quat &out) {
  if (node_anim->rotation_keys.size() == 1) {
    out.w = node_anim->rotation_keys[0].value.w;
    out.x = node_anim->rotation_keys[0].value.x;
    out.y = node_anim->rotation_keys[0].value.y;
    out.z = node_anim->rotation_keys[0].value.z;
    return;
  }

  u32 rotation_index = this->find_rotation(animation_time, node_anim);
  u32 next_rotation_index = (rotation_index + 1);
  assert(next_rotation_index < node_anim->rotation_keys.size());
  float delta_time =
      (float)(node_anim->rotation_keys[next_rotation_index].time - node_anim->rotation_keys[rotation_index].time);
  float factor = (animation_time - (float)node_anim->rotation_keys[rotation_index].time) / delta_time;

  if (factor < 0.0f)
    factor = 0.0f;
  if (factor > 1.0f)
    factor = 1.0f;

  const glm::quat &start_glm = node_anim->rotation_keys[rotation_index].value;
  const glm::quat &end_glm = node_anim->rotation_keys[next_rotation_index].value;

  out = glm::slerp(start_glm, end_glm, factor);
  out = glm::normalize(out);

  return;
}

void SkinnedMesh::calculate_glm_interpolated_position(float animation_time,
                                                      const runtime::animation::Channel *node_anim, glm::vec3 &out) {
  if (node_anim->position_keys.size() == 1) {
    out.x = node_anim->position_keys[0].value.x;
    out.y = node_anim->position_keys[0].value.y;
    out.z = node_anim->position_keys[0].value.z;
    return;
  }

  u32 position_index = this->find_position(animation_time, node_anim);
  u32 next_position_index = (position_index + 1);
  assert(next_position_index < node_anim->position_keys.size());
  float delta_time =
      (float)(node_anim->position_keys[next_position_index].time - node_anim->position_keys[position_index].time);
  float factor = (animation_time - (float)node_anim->position_keys[position_index].time) / delta_time;
  if (factor < 0.0f)
    factor = 0.0f;
  if (factor > 1.0f)
    factor = 1.0f;
  const glm::vec3 &start = node_anim->position_keys[position_index].value;
  const glm::vec3 &end = node_anim->position_keys[next_position_index].value;
  out = (end - start) * factor + start;
  return;
}

u32 SkinnedMesh::find_rotation(float animation_time, const runtime::animation::Channel *node_anim) {
  for (u32 rotation_key_index = 0; rotation_key_index != node_anim->rotation_keys.size() - 1; rotation_key_index++) {
    if (animation_time < (float)node_anim->rotation_keys[rotation_key_index + 1].time) {
      return rotation_key_index;
    }
  }

  return 0;
}

u32 SkinnedMesh::find_position(float animation_time, const runtime::animation::Channel *node_anim) {
  for (u32 PositionKeyIndex = 0; PositionKeyIndex != node_anim->position_keys.size() - 1; PositionKeyIndex++) {
    if (animation_time < (float)node_anim->position_keys[PositionKeyIndex + 1].time) {
      return PositionKeyIndex;
    }
  }
  return 0;
}

u32 SkinnedMesh::find_scaling(float animation_time, const runtime::animation::Channel *node_anim) {
  for (u32 scaling_key_index = 0; scaling_key_index != node_anim->scaling_keys.size() - 1; scaling_key_index++) {
    if (animation_time < (float)node_anim->scaling_keys[scaling_key_index + 1].time) {
      return scaling_key_index;
    }
  }

  return 0;
}
