#include <runtime/skinned_mesh.h>
#include <assimp/scene.h>

using namespace runtime;

namespace gi = graphicsinterface;
using gi::PixelFormat;

SkinnedMeshResource::SkinnedMeshResource() {
  namespace gi = graphicsinterface;
  vertex_stream.count = 0;
  vertex_stream.add({0, gi::SemanticName::Position, gi::PixelFormat::FloatRGBA, 0u, 0u});
  // vertex_stream.add({0, gi::SemanticName::Color, gi::PixelFormat::FloatRGBA, 16, 0u});
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
  std::map<String /*animation FRIENDLY name*/, AnimationInfo>::iterator itAnimation =
      this->animation_name_to_pscene.find(animationName);

  // Found it?
  if (itAnimation == this->animation_name_to_pscene.end()) { // Nope.
    return 0.0f;
  }

  // This is scaling the animation from 0 to 1
  return (float)itAnimation->second.animation.duration / (float)itAnimation->second.animation.ticks_per_second;
}

void SkinnedMesh::bone_transform(float time_in_seconds,
                                 String animationName, // Now we can pick the animation
                                 std::vector<glm::mat4> &final_transformation, std::vector<glm::mat4> &globals,
                                 std::vector<glm::mat4> &offsets) {
  glm::mat4 ident(1.0f);
  const aiScene *p_scene = (const aiScene *)pScene;

  float ticks_per_second = static_cast<float>(
      p_scene->mAnimations[0]->mTicksPerSecond != 0 ? p_scene->mAnimations[0]->mTicksPerSecond : 25.0);

  float time_in_ticks = time_in_seconds * ticks_per_second;
  float animation_time = fmod(time_in_ticks, find_animation_total_time(animationName));

  // use the "animation" file to look up these nodes
  // (need the matOffset information from the animation file)
  this->read_node_hierarchy(animation_time, animationName, p_scene->mRootNode, ident);

  final_transformation.resize(this->number_of_bones);
  globals.resize(this->number_of_bones);
  offsets.resize(this->number_of_bones);

  for (u32 bone_index = 0; bone_index < this->number_of_bones; bone_index++) {
    final_transformation[bone_index] = glm::transpose(this->bone_info[bone_index].final_transformation);
    globals[bone_index] = this->bone_info[bone_index].ObjectBoneTransformation;
    offsets[bone_index] = this->bone_info[bone_index].BoneOffset;
  }
}
// Looks in the animation map and returns the total time
float SkinnedMesh::find_animation_total_time(String animation_name) {
  Map<String, AnimationInfo>::iterator itAnimation = this->animation_name_to_pscene.find(animation_name);

  // Found it?
  if (itAnimation == this->animation_name_to_pscene.end()) { // Nope.
    return 0.0f;
  }
  // This is scaling the animation from 0 to 1
  return (float)itAnimation->second.animation.duration;
}

glm::mat4 ai_matrix_to_glm_matrix(const aiMatrix4x4 &mat) {
  return glm::mat4(                   //
      mat.a1, mat.b1, mat.c1, mat.d1, //
      mat.a2, mat.b2, mat.c2, mat.d2, //
      mat.a3, mat.b3, mat.c3, mat.d3, //
      mat.a4, mat.b4, mat.c4, mat.d4  //
  );
}

const aiNodeAnim *SkinnedMesh::find_node_animation_channel(runtime::animation::Animation *pAnimation, const String &boneName) {
  for (u32 ChannelIndex = 0; ChannelIndex != pAnimation->mNumChannels; ChannelIndex++) {
    if (pAnimation->mChannels[ChannelIndex]->mNodeName == aiString(boneName)) {
      return pAnimation->mChannels[ChannelIndex];
    }
  }
  return 0;
}

void SkinnedMesh::calculate_glm_interpolated_scaling(float animation_time, const aiNodeAnim *node_anim,
                                                     glm::vec3 &out) {
  if (node_anim->mNumScalingKeys == 1) {
    out.x = node_anim->mScalingKeys[0].mValue.x;
    out.y = node_anim->mScalingKeys[0].mValue.y;
    out.z = node_anim->mScalingKeys[0].mValue.z;
    return;
  }

  u32 scaling_index = this->find_scaling(animation_time, node_anim);
  u32 next_scaling_index = (scaling_index + 1);
  assert(next_scaling_index < node_anim->mNumScalingKeys);
  float delta_time =
      (float)(node_anim->mScalingKeys[next_scaling_index].mTime - node_anim->mScalingKeys[scaling_index].mTime);
  float factor = (animation_time - (float)node_anim->mScalingKeys[scaling_index].mTime) / delta_time;

  if (factor < 0.0f)
    factor = 0.0f;
  if (factor > 1.0f)
    factor = 1.0f;

  const aiVector3D &start_scale = node_anim->mScalingKeys[scaling_index].mValue;
  const aiVector3D &end_scale = node_anim->mScalingKeys[next_scaling_index].mValue;
  glm::vec3 start = glm::vec3(start_scale.x, start_scale.y, start_scale.z);
  glm::vec3 end = glm::vec3(end_scale.x, end_scale.y, end_scale.z);

  out = (end - start) * factor + start;
  return;
}

void SkinnedMesh::read_node_hierarchy(float animation_time, String animation_name, const aiNode *pNode,
                                      const glm::mat4 &ParentTransformMatrix) {
  aiString node_name(pNode->mName.data);

  auto scene = (const aiScene *)this->pScene;
  // Original version picked the "main scene" animation...
  //const aiAnimation *pAnimation = scene->mAnimations[0];
  runtime::animation::Animation *pAnimation = nullptr;
  Map<String, AnimationInfo>::iterator itAnimation = this->animation_name_to_pscene.find(animation_name); // Animations

  // Did we find it?
  if (itAnimation != this->animation_name_to_pscene.end()) {
    // const aiAnimation* ascene = (const aiAnimation *)itAnimation->second.ai_animation;
    // pAnimation = reinterpret_cast<const aiAnimation *>(ascene);
    pAnimation =& itAnimation->second.animation;
  }

  // Transformation of the node in bind pose
  glm::mat4 node_transform = ai_matrix_to_glm_matrix(pNode->mTransformation);
  const aiNodeAnim *node_anim = this->find_node_animation_channel(pAnimation, node_name.C_Str());
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

  std::map<String, u32>::iterator it = this->bone_name_to_bone_index.find(String(node_name.data));
  if (it != this->bone_name_to_bone_index.end()) {
    u32 BoneIndex = it->second;
    this->bone_info[BoneIndex].ObjectBoneTransformation = object_bone_transform;
    this->bone_info[BoneIndex].final_transformation =
        this->global_inverse_transformation * object_bone_transform * this->bone_info[BoneIndex].BoneOffset;

  } else {
    int breakpoint = 0;
  }

  for (u32 child_index = 0; child_index != pNode->mNumChildren; child_index++) {
    this->read_node_hierarchy(animation_time, animation_name, pNode->mChildren[child_index], object_bone_transform);
  }
}

void SkinnedMesh::calculate_glm_interpolated_rotation(float animation_time, const aiNodeAnim *node_anim,
                                                      glm::quat &out) {
  if (node_anim->mNumRotationKeys == 1) {
    out.w = node_anim->mRotationKeys[0].mValue.w;
    out.x = node_anim->mRotationKeys[0].mValue.x;
    out.y = node_anim->mRotationKeys[0].mValue.y;
    out.z = node_anim->mRotationKeys[0].mValue.z;
    return;
  }

  u32 rotation_index = this->find_rotation(animation_time, node_anim);
  u32 next_rotation_index = (rotation_index + 1);
  assert(next_rotation_index < node_anim->mNumRotationKeys);
  float delta_time =
      (float)(node_anim->mRotationKeys[next_rotation_index].mTime - node_anim->mRotationKeys[rotation_index].mTime);
  float factor = (animation_time - (float)node_anim->mRotationKeys[rotation_index].mTime) / delta_time;

  if (factor < 0.0f)
    factor = 0.0f;
  if (factor > 1.0f)
    factor = 1.0f;

  const aiQuaternion &start_rotation_q = node_anim->mRotationKeys[rotation_index].mValue;
  const aiQuaternion &end_rotation_q = node_anim->mRotationKeys[next_rotation_index].mValue;

  glm::quat start_glm = glm::quat(start_rotation_q.w, start_rotation_q.x, start_rotation_q.y, start_rotation_q.z);
  glm::quat end_glm = glm::quat(end_rotation_q.w, end_rotation_q.x, end_rotation_q.y, end_rotation_q.z);

  out = glm::slerp(start_glm, end_glm, factor);
  out = glm::normalize(out);

  return;
}

void SkinnedMesh::calculate_glm_interpolated_position(float animation_time, const aiNodeAnim *node_anim,
                                                      glm::vec3 &out) {
  if (node_anim->mNumPositionKeys == 1) {
    out.x = node_anim->mPositionKeys[0].mValue.x;
    out.y = node_anim->mPositionKeys[0].mValue.y;
    out.z = node_anim->mPositionKeys[0].mValue.z;
    return;
  }

  u32 position_index = this->find_position(animation_time, node_anim);
  u32 next_position_index = (position_index + 1);
  assert(next_position_index < node_anim->mNumPositionKeys);
  float delta_time =
      (float)(node_anim->mPositionKeys[next_position_index].mTime - node_anim->mPositionKeys[position_index].mTime);
  float factor = (animation_time - (float)node_anim->mPositionKeys[position_index].mTime) / delta_time;
  if (factor < 0.0f)
    factor = 0.0f;
  if (factor > 1.0f)
    factor = 1.0f;
  const aiVector3D &start_position = node_anim->mPositionKeys[position_index].mValue;
  const aiVector3D &end_position = node_anim->mPositionKeys[next_position_index].mValue;
  glm::vec3 start = glm::vec3(start_position.x, start_position.y, start_position.z);
  glm::vec3 end = glm::vec3(end_position.x, end_position.y, end_position.z);

  out = (end - start) * factor + start;

  return;
}

u32 SkinnedMesh::find_rotation(float animation_time, const aiNodeAnim *node_anim) {
  for (u32 rotation_key_index = 0; rotation_key_index != node_anim->mNumRotationKeys - 1; rotation_key_index++) {
    if (animation_time < (float)node_anim->mRotationKeys[rotation_key_index + 1].mTime) {
      return rotation_key_index;
    }
  }

  return 0;
}

u32 SkinnedMesh::find_position(float animation_time, const aiNodeAnim *node_anim) {
  for (u32 PositionKeyIndex = 0; PositionKeyIndex != node_anim->mNumPositionKeys - 1; PositionKeyIndex++) {
    if (animation_time < (float)node_anim->mPositionKeys[PositionKeyIndex + 1].mTime) {
      return PositionKeyIndex;
    }
  }
  return 0;
}

u32 SkinnedMesh::find_scaling(float animation_time, const aiNodeAnim *node_anim) {
  for (u32 scaling_key_index = 0; scaling_key_index != node_anim->mNumScalingKeys - 1; scaling_key_index++) {
    if (animation_time < (float)node_anim->mScalingKeys[scaling_key_index + 1].mTime) {
      return scaling_key_index;
    }
  }

  return 0;
}
