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
    // init loading stuff
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

    usize stride = sizeof(sVertex_xyz_rgba_n_uv2_bt_4Bones);
    render_data->vertex_buffer = gi::create_vertex_buffer(vertices.size(), stride, PixelFormat::FloatRGBA);
    usize vertex_byte_size = vertices.size() * stride;
    gi::set_vertex_buffer_data(vertices.data(), vertex_byte_size, render_data->vertex_buffer);
    free();

    render_data->vertex_stream.add_buffer(render_data->vertex_buffer.get());
  }

  return render_data.get();
}

float SkinnedMesh::get_duration_seconds(std::string animationName) {
  // std::map< std::string /*animationfile*/,
  //	const aiScene* >::iterator itAnimation = this->mapAnimationNameTo_pScene.find(animationName);
  std::map<std::string /*animation FRIENDLY name*/, sAnimationInfo>::iterator itAnimation =
      this->mapAnimationFriendlyNameTo_pScene.find(animationName);

  // Found it?
  if (itAnimation == this->mapAnimationFriendlyNameTo_pScene.end()) { // Nope.
    return 0.0f;
  }

  // This is scaling the animation from 0 to 1

  auto scene = (const aiScene *)itAnimation->second.pAIScene;
  return (float)scene->mAnimations[0]->mDuration / (float)scene->mAnimations[0]->mTicksPerSecond;
}

void SkinnedMesh::bone_transform(float TimeInSeconds,
                                 std::string animationName, // Now we can pick the animation
                                 std::vector<glm::mat4> &FinalTransformation, std::vector<glm::mat4> &Globals,
                                 std::vector<glm::mat4> &Offsets) {
  glm::mat4 Identity(1.0f);
  const aiScene *p_scene = (const aiScene *)pScene;

  float TicksPerSecond = static_cast<float>(
      p_scene->mAnimations[0]->mTicksPerSecond != 0 ? p_scene->mAnimations[0]->mTicksPerSecond : 25.0);

  float TimeInTicks = TimeInSeconds * TicksPerSecond;
  float AnimationTime = fmod(TimeInTicks, FindAnimationTotalTime(animationName));

  // use the "animation" file to look up these nodes
  // (need the matOffset information from the animation file)
  this->ReadNodeHeirarchy(AnimationTime, animationName, p_scene->mRootNode, Identity);

  FinalTransformation.resize(this->mNumBones);
  Globals.resize(this->mNumBones);
  Offsets.resize(this->mNumBones);

  for (unsigned int BoneIndex = 0; BoneIndex < this->mNumBones; BoneIndex++) {
    FinalTransformation[BoneIndex] = glm::transpose( this->mBoneInfo[BoneIndex].FinalTransformation);
    Globals[BoneIndex] = this->mBoneInfo[BoneIndex].ObjectBoneTransformation;
    Offsets[BoneIndex] = this->mBoneInfo[BoneIndex].BoneOffset;
  }
}
// Looks in the animation map and returns the total time
float SkinnedMesh::FindAnimationTotalTime(std::string animationName) {
  // std::map< std::string /*animationfile*/,
  //	const aiScene* >::iterator itAnimation = this->mapAnimationNameTo_pScene.find(animationName);
  std::map<std::string /*animation FRIENDLY name*/, sAnimationInfo>::iterator itAnimation =
      this->mapAnimationFriendlyNameTo_pScene.find(animationName);

  // Found it?
  if (itAnimation == this->mapAnimationFriendlyNameTo_pScene.end()) { // Nope.
    return 0.0f;
  }
  auto scene = (const aiScene *)itAnimation->second.pAIScene;

  // This is scaling the animation from 0 to 1
  return (float)scene->mAnimations[0]->mDuration;
}

glm::mat4 AIMatrixToGLMMatrix(const aiMatrix4x4 &mat) {
  return glm::mat4(mat.a1, mat.b1, mat.c1, mat.d1, mat.a2, mat.b2, mat.c2, mat.d2, mat.a3, mat.b3, mat.c3, mat.d3,
                   mat.a4, mat.b4, mat.c4, mat.d4);
}

const aiNodeAnim *SkinnedMesh::FindNodeAnimationChannel(const aiAnimation *pAnimation, const String& boneName) {
  for (unsigned int ChannelIndex = 0; ChannelIndex != pAnimation->mNumChannels; ChannelIndex++) {
    if (pAnimation->mChannels[ChannelIndex]->mNodeName == aiString( boneName)) {
      return pAnimation->mChannels[ChannelIndex];
    }
  }
  return 0;
}

void SkinnedMesh::CalcGLMInterpolatedScaling(float AnimationTime, const aiNodeAnim *pNodeAnim, glm::vec3 &out) {
  if (pNodeAnim->mNumScalingKeys == 1) {
    out.x = pNodeAnim->mScalingKeys[0].mValue.x;
    out.y = pNodeAnim->mScalingKeys[0].mValue.y;
    out.z = pNodeAnim->mScalingKeys[0].mValue.z;
    return;
  }

  unsigned int ScalingIndex = this->FindScaling(AnimationTime, pNodeAnim);
  unsigned int NextScalingIndex = (ScalingIndex + 1);
  assert(NextScalingIndex < pNodeAnim->mNumScalingKeys);
  float DeltaTime =
      (float)(pNodeAnim->mScalingKeys[NextScalingIndex].mTime - pNodeAnim->mScalingKeys[ScalingIndex].mTime);
  float Factor = (AnimationTime - (float)pNodeAnim->mScalingKeys[ScalingIndex].mTime) / DeltaTime;
  if (Factor < 0.0f)
    Factor = 0.0f;
  if (Factor > 1.0f)
    Factor = 1.0f;
  const aiVector3D &StartScale = pNodeAnim->mScalingKeys[ScalingIndex].mValue;
  const aiVector3D &EndScale = pNodeAnim->mScalingKeys[NextScalingIndex].mValue;
  glm::vec3 start = glm::vec3(StartScale.x, StartScale.y, StartScale.z);
  glm::vec3 end = glm::vec3(EndScale.x, EndScale.y, EndScale.z);
  out = (end - start) * Factor + start;

  return;
}

void SkinnedMesh::ReadNodeHeirarchy(float AnimationTime, std::string animationName, const aiNode *pNode,
                                    const glm::mat4 &ParentTransformMatrix) {
  aiString NodeName(pNode->mName.data);

  auto scene = (const aiScene *)this->pScene;

  // Original version picked the "main scene" animation...
  const aiAnimation *pAnimation = scene->mAnimations[0];

  // Search for the animation we want...
  //	std::map< std::string, const aiScene* >::iterator itAnimation
  //					= mapAnimationNameTo_pScene.find(animationName);
  std::map<std::string /*animation FRIENDLY name*/,
           sAnimationInfo>::iterator itAnimation =
      this->mapAnimationFriendlyNameTo_pScene.find(animationName); // Animations

  // Did we find it?
  if (itAnimation != this->mapAnimationFriendlyNameTo_pScene.end()) {
    // Yes, there is an animation called that...
    // ...replace the animation with the one we found
    auto ascene = (const aiScene *)itAnimation->second.pAIScene;

    pAnimation = reinterpret_cast<const aiAnimation *>(ascene->mAnimations[0]);
  }

  // aiMatrix4x4 NodeTransformation;

  // Transformation of the node in bind pose
  glm::mat4 NodeTransformation = AIMatrixToGLMMatrix(pNode->mTransformation);

  const aiNodeAnim *pNodeAnim = this->FindNodeAnimationChannel(pAnimation, NodeName.C_Str());

  if (pNodeAnim) {
    // Get interpolated scaling
    glm::vec3 scale;
    this->CalcGLMInterpolatedScaling(AnimationTime, pNodeAnim, scale);
    glm::mat4 ScalingM = glm::scale(glm::mat4(1.0f), scale);

    // Get interpolated rotation (quaternion)
    glm::quat ori;
    this->CalcGLMInterpolatedRotation(AnimationTime, pNodeAnim, ori);
    glm::mat4 RotationM = glm::mat4_cast(ori);

    // Get interpolated position
    glm::vec3 pos;
    this->CalcGLMInterpolatedPosition(AnimationTime, pNodeAnim, pos);
    glm::mat4 TranslationM = glm::translate(glm::mat4(1.0f), pos);

    // Combine the above transformations
    NodeTransformation = TranslationM * RotationM * ScalingM;
  }

  glm::mat4 ObjectBoneTransformation = ParentTransformMatrix * NodeTransformation;

  std::map<std::string, unsigned int>::iterator it = this->m_mapBoneNameToBoneIndex.find(std::string(NodeName.data));
  if (it != this->m_mapBoneNameToBoneIndex.end()) {
    unsigned int BoneIndex = it->second;
    this->mBoneInfo[BoneIndex].ObjectBoneTransformation = ObjectBoneTransformation;
    this->mBoneInfo[BoneIndex].FinalTransformation =
        this->mGlobalInverseTransformation * ObjectBoneTransformation * this->mBoneInfo[BoneIndex].BoneOffset;

  } else {
    int breakpoint = 0;
  }

  for (unsigned int ChildIndex = 0; ChildIndex != pNode->mNumChildren; ChildIndex++) {
    this->ReadNodeHeirarchy(AnimationTime, animationName, pNode->mChildren[ChildIndex], ObjectBoneTransformation);
  }
}

void SkinnedMesh::CalcGLMInterpolatedRotation(float AnimationTime, const aiNodeAnim *pNodeAnim, glm::quat &out) {
  if (pNodeAnim->mNumRotationKeys == 1) {
    out.w = pNodeAnim->mRotationKeys[0].mValue.w;
    out.x = pNodeAnim->mRotationKeys[0].mValue.x;
    out.y = pNodeAnim->mRotationKeys[0].mValue.y;
    out.z = pNodeAnim->mRotationKeys[0].mValue.z;
    return;
  }

  unsigned int RotationIndex = this->FindRotation(AnimationTime, pNodeAnim);
  unsigned int NextRotationIndex = (RotationIndex + 1);
  assert(NextRotationIndex < pNodeAnim->mNumRotationKeys);
  float DeltaTime =
      (float)(pNodeAnim->mRotationKeys[NextRotationIndex].mTime - pNodeAnim->mRotationKeys[RotationIndex].mTime);
  float Factor = (AnimationTime - (float)pNodeAnim->mRotationKeys[RotationIndex].mTime) / DeltaTime;
  if (Factor < 0.0f)
    Factor = 0.0f;
  if (Factor > 1.0f)
    Factor = 1.0f;
  const aiQuaternion &StartRotationQ = pNodeAnim->mRotationKeys[RotationIndex].mValue;
  const aiQuaternion &EndRotationQ = pNodeAnim->mRotationKeys[NextRotationIndex].mValue;

  glm::quat StartGLM = glm::quat(StartRotationQ.w, StartRotationQ.x, StartRotationQ.y, StartRotationQ.z);
  glm::quat EndGLM = glm::quat(EndRotationQ.w, EndRotationQ.x, EndRotationQ.y, EndRotationQ.z);

  out = glm::slerp(StartGLM, EndGLM, Factor);

  out = glm::normalize(out);

  return;
}

void SkinnedMesh::CalcGLMInterpolatedPosition(float AnimationTime, const aiNodeAnim *pNodeAnim, glm::vec3 &out) {
  if (pNodeAnim->mNumPositionKeys == 1) {
    out.x = pNodeAnim->mPositionKeys[0].mValue.x;
    out.y = pNodeAnim->mPositionKeys[0].mValue.y;
    out.z = pNodeAnim->mPositionKeys[0].mValue.z;
    return;
  }

  unsigned int PositionIndex = this->FindPosition(AnimationTime, pNodeAnim);
  unsigned int NextPositionIndex = (PositionIndex + 1);
  assert(NextPositionIndex < pNodeAnim->mNumPositionKeys);
  float DeltaTime =
      (float)(pNodeAnim->mPositionKeys[NextPositionIndex].mTime - pNodeAnim->mPositionKeys[PositionIndex].mTime);
  float Factor = (AnimationTime - (float)pNodeAnim->mPositionKeys[PositionIndex].mTime) / DeltaTime;
  if (Factor < 0.0f)
    Factor = 0.0f;
  if (Factor > 1.0f)
    Factor = 1.0f;
  const aiVector3D &StartPosition = pNodeAnim->mPositionKeys[PositionIndex].mValue;
  const aiVector3D &EndPosition = pNodeAnim->mPositionKeys[NextPositionIndex].mValue;
  glm::vec3 start = glm::vec3(StartPosition.x, StartPosition.y, StartPosition.z);
  glm::vec3 end = glm::vec3(EndPosition.x, EndPosition.y, EndPosition.z);

  out = (end - start) * Factor + start;

  return;
}

unsigned int SkinnedMesh::FindRotation(float AnimationTime, const aiNodeAnim *pNodeAnim) {
  for (unsigned int RotationKeyIndex = 0; RotationKeyIndex != pNodeAnim->mNumRotationKeys - 1; RotationKeyIndex++) {
    if (AnimationTime < (float)pNodeAnim->mRotationKeys[RotationKeyIndex + 1].mTime) {
      return RotationKeyIndex;
    }
  }

  return 0;
}

unsigned int SkinnedMesh::FindPosition(float AnimationTime, const aiNodeAnim *pNodeAnim) {
  for (unsigned int PositionKeyIndex = 0; PositionKeyIndex != pNodeAnim->mNumPositionKeys - 1; PositionKeyIndex++) {
    if (AnimationTime < (float)pNodeAnim->mPositionKeys[PositionKeyIndex + 1].mTime) {
      return PositionKeyIndex;
    }
  }

  return 0;
}

unsigned int SkinnedMesh::FindScaling(float AnimationTime, const aiNodeAnim *pNodeAnim) {
  for (unsigned int ScalingKeyIndex = 0; ScalingKeyIndex != pNodeAnim->mNumScalingKeys - 1; ScalingKeyIndex++) {
    if (AnimationTime < (float)pNodeAnim->mScalingKeys[ScalingKeyIndex + 1].mTime) {
      return ScalingKeyIndex;
    }
  }

  return 0;
}
