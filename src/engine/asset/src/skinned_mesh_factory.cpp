#include <asset/skinned_mesh_factory.h>
#include <assimp/cimport.h>
#include <assimp/mesh.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>

#include <config/config.h>

#include <rapidjson/document.h>
#include <utils/fs.h>

using runtime::SkinnedMesh;
using namespace glm;
using runtime::AnimationInfo;

using runtime::SkinnedMeshVertex;

mat4 ai_matrix_to_gl_matrix(const aiMatrix4x4 &mat) {
  return mat4(mat.a1, mat.b1, mat.c1, mat.d1, mat.a2, mat.b2, mat.c2, mat.d2, mat.a3, mat.b3, mat.c3, mat.d3, mat.a4,
              mat.b4, mat.c4, mat.d4);
}

namespace runtime {
void VertexBoneData::AddBoneData(u32 BoneID, float Weight) {
  for (i32 Index = 0; Index < sizeof(this->ids) / sizeof(this->ids[0]); Index++) {
    if (this->weights[Index] == 0.0f) {
      this->ids[Index] = (float)BoneID;
      this->weights[Index] = Weight;
      return;
    }
  }
}
} // namespace runtime
void SkinnedMeshFactory::load_bones(aiMesh *Mesh, Vector<runtime::VertexBoneData> &vertexBoneData,
                                    runtime::SkinnedMesh &mesh_sm) {
  for (i32 boneIndex = 0; boneIndex != Mesh->mNumBones; boneIndex++) {
    i32 BoneIndex = 0;
    String BoneName(Mesh->mBones[boneIndex]->mName.data);

    Map<String, u32>::iterator it = mesh_sm.bone_name_to_bone_index.find(BoneName);
    if (it == mesh_sm.bone_name_to_bone_index.end()) {
      BoneIndex = mesh_sm.number_of_bones;
      mesh_sm.number_of_bones++;
      runtime::sBoneInfo bi;
      mesh_sm.bone_info.push_back(bi);

      mesh_sm.bone_info[BoneIndex].BoneOffset = ai_matrix_to_gl_matrix(Mesh->mBones[boneIndex]->mOffsetMatrix);
      mesh_sm.bone_name_to_bone_index[BoneName] = BoneIndex;
    } else {
      BoneIndex = it->second;
    }

    for (i32 WeightIndex = 0; WeightIndex != Mesh->mBones[boneIndex]->mNumWeights; WeightIndex++) {
      i32 VertexID =
          /*mMeshEntries[MeshIndex].BaseVertex +*/ Mesh->mBones[boneIndex]->mWeights[WeightIndex].mVertexId;
      float Weight = Mesh->mBones[boneIndex]->mWeights[WeightIndex].mWeight;
      vertexBoneData[VertexID].AddBoneData(BoneIndex, Weight);
    }
  }
}

inline String import_path(const String &raw_path) {
  return utils::path::join(config::CONTENT_DIR, raw_path);
}

void load_animation(runtime::animation::Animation &animation, aiAnimation *ai_animation) {
  assert(ai_animation);
  auto &ai = *ai_animation;
  animation.name = ai.mName.C_Str();
  animation.duration = ai.mDuration;
  animation.ticks_per_second = ai.mTicksPerSecond;
  animation.channels.resize(ai.mNumChannels);
  animation.mesh_channels.resize(ai.mNumMeshChannels);

  for (i32 channel_index = 0; channel_index < animation.channels.size(); channel_index++) {
    auto &node_anim = *ai.mChannels[channel_index];
    auto &ch = animation.channels[channel_index];
    ch.node_name = node_anim.mNodeName.C_Str();

    ch.post_state = (runtime::animation::AnimationBehaviour)node_anim.mPostState;
    ch.pre_state = (runtime::animation::AnimationBehaviour)node_anim.mPreState;

    ch.position_keys.resize(node_anim.mNumPositionKeys);
    ch.rotation_keys.resize(node_anim.mNumRotationKeys);
    ch.scaling_keys.resize(node_anim.mNumScalingKeys);

    for (i32 x = 0; x < ch.position_keys.size(); x++) {
      ch.position_keys[x].time = node_anim.mPositionKeys[x].mTime;
      ch.position_keys[x].value.x = node_anim.mPositionKeys[x].mValue.x;
      ch.position_keys[x].value.y = node_anim.mPositionKeys[x].mValue.y;
      ch.position_keys[x].value.z = node_anim.mPositionKeys[x].mValue.z;
    }

    for (i32 x = 0; x < ch.rotation_keys.size(); x++) {
      ch.rotation_keys[x].time = node_anim.mRotationKeys[x].mTime;
      ch.rotation_keys[x].value.w = node_anim.mRotationKeys[x].mValue.w;
      ch.rotation_keys[x].value.x = node_anim.mRotationKeys[x].mValue.x;
      ch.rotation_keys[x].value.y = node_anim.mRotationKeys[x].mValue.y;
      ch.rotation_keys[x].value.z = node_anim.mRotationKeys[x].mValue.z;
    }

    for (i32 x = 0; x < ch.scaling_keys.size(); x++) {
      ch.scaling_keys[x].time = node_anim.mScalingKeys[x].mTime;
      ch.scaling_keys[x].value.x = node_anim.mScalingKeys[x].mValue.x;
      ch.scaling_keys[x].value.y = node_anim.mScalingKeys[x].mValue.y;
      ch.scaling_keys[x].value.z = node_anim.mScalingKeys[x].mValue.z;
    }
  }

  for (i32 mesh_channel_index = 0; mesh_channel_index < animation.mesh_channels.size(); mesh_channel_index++) {
    auto &mesh_channel = animation.mesh_channels[mesh_channel_index];
    auto &ai_mesh_ch = ai.mMeshChannels[mesh_channel_index];

    mesh_channel.name = ai_mesh_ch->mName.C_Str();
    mesh_channel.keys.resize(ai_mesh_ch->mNumKeys);

    for (i32 x = 0; x < mesh_channel.keys.size(); x++) {
      mesh_channel.keys[x].time = ai_mesh_ch->mKeys[x].mTime;
      mesh_channel.keys[x].value = ai_mesh_ch->mKeys[x].mValue;
    }
  }
}

bool SkinnedMeshFactory::load_mesh_animation(const String &friendly_name, const String &filename,
                                             runtime::SkinnedMesh &mesh, bool has_exit_time) // Only want animations
{
  Map<String, AnimationInfo>::iterator it_animation = mesh.animation_name_to_pscene.find(friendly_name);

  if (it_animation != mesh.animation_name_to_pscene.end()) {
    return false;
  }

  i32 flags =
      aiProcess_Triangulate | aiProcess_OptimizeMeshes | aiProcess_OptimizeGraph | aiProcess_JoinIdenticalVertices;

  Assimp::Importer *importer = new Assimp::Importer();
  AnimationInfo anim_info;
  //anim_info.friendly_name = friendly_name;
  //anim_info.filename = filename;
  const aiScene *ai_scene = importer->ReadFile(filename.c_str(), flags);
  //anim_info.has_exit_time = has_exit_time;

  if (!ai_scene)
    return false;

  load_animation(anim_info.animation, ai_scene->mAnimations[0]);

  //anim_info.ai_animation = ai_scene->mAnimations[0];
  //anim_info.duration = (float)(anim_info.ai_animation->mDuration / anim_info.ai_animation->mTicksPerSecond);
  //mesh.animation_name_to_pscene[anim_info.friendly_name] = anim_info;
  return true;
}

void SkinnedMeshFactory::load_asset_data(asset::Asset &asset) {

  SkinnedMesh &mesh = *reinterpret_cast<SkinnedMesh *>(&asset);
  i32 flags = aiProcess_Triangulate | aiProcess_OptimizeMeshes | aiProcess_OptimizeGraph |
              aiProcess_JoinIdenticalVertices | aiProcess_GenNormals | aiProcess_CalcTangentSpace;

  struct AnimationNameFilename {
    String name;
    String filename;
  };

  mesh.number_of_vertices = 0;
  utils::free_vector(mesh.vertex_bone_data);

  String mesh_filename;
  Vector<AnimationNameFilename> animation_filenames;
  {
    using namespace rapidjson;
    Document document;
    String json_content = utils::fs::load_file_to_string(asset.source_filename);
    assert(!document.Parse<kParseStopWhenDoneFlag>(json_content.c_str()).HasParseError());
    assert(document.HasMember("mesh"));
    assert(document.HasMember("animations"));

    mesh_filename = import_path(document["mesh"].GetString());

    const Value &animations_json = document["animations"];
    for (Value::ConstMemberIterator it = animations_json.MemberBegin(); it != animations_json.MemberEnd(); ++it) {
      printf("%s\t", it->name.GetString());
      animation_filenames.push_back({it->name.GetString(), import_path(it->value.GetString())});
    }
  }

  assert(mesh_filename.size());
  p_scene = aiImportFile(mesh_filename.c_str(), flags);

  assert(p_scene);

  mesh.global_inverse_transformation = ai_matrix_to_gl_matrix(p_scene->mRootNode->mTransformation);
  mesh.global_inverse_transformation = inverse(mesh.global_inverse_transformation);

  mesh.number_of_vertices = p_scene->mMeshes[0]->mNumVertices;
  // This is the vertex information for JUST the bone stuff
  mesh.vertex_bone_data.resize(mesh.number_of_vertices);

  this->load_bones(this->p_scene->mMeshes[0], mesh.vertex_bone_data, mesh);
  String default_anim = "runForward";

  for (auto &animation : animation_filenames) {
    if (default_anim == "")
      default_anim = animation.name;
    load_mesh_animation(animation.name, animation.filename, mesh);
  }

  {
    // Copy the data from assimp format into the sModelDrawInfo format...
    i32 number_of_vertices = p_scene->mMeshes[0]->mNumVertices;
    i32 number_of_triangles = p_scene->mMeshes[0]->mNumFaces;
    i32 number_of_indices = number_of_triangles * 3;

    mesh.vertices.resize(number_of_vertices);

    for (i32 vert_index = 0; vert_index != number_of_vertices; vert_index++) {
      mesh.vertices[vert_index].x = p_scene->mMeshes[0]->mVertices[vert_index].x;
      mesh.vertices[vert_index].y = p_scene->mMeshes[0]->mVertices[vert_index].y;
      mesh.vertices[vert_index].z = p_scene->mMeshes[0]->mVertices[vert_index].z;

      // Normals...
      mesh.vertices[vert_index].nx = p_scene->mMeshes[0]->mNormals[vert_index].x;
      mesh.vertices[vert_index].ny = p_scene->mMeshes[0]->mNormals[vert_index].y;
      mesh.vertices[vert_index].nz = p_scene->mMeshes[0]->mNormals[vert_index].z;

      // bi-normal  (or bi-tangent)
      mesh.vertices[vert_index].bx = p_scene->mMeshes[0]->mBitangents[vert_index].x;
      mesh.vertices[vert_index].by = p_scene->mMeshes[0]->mBitangents[vert_index].y;
      mesh.vertices[vert_index].bz = p_scene->mMeshes[0]->mBitangents[vert_index].z;

      // Tangent
      mesh.vertices[vert_index].tx = p_scene->mMeshes[0]->mTangents[vert_index].x;
      mesh.vertices[vert_index].ty = p_scene->mMeshes[0]->mTangents[vert_index].y;
      mesh.vertices[vert_index].tz = p_scene->mMeshes[0]->mTangents[vert_index].z;

      // uv2 (which are odd in assimp)
      if (p_scene->mMeshes[0]->HasTextureCoords(0)) { // 1st UV coords
        mesh.vertices[vert_index].u0 = p_scene->mMeshes[0]->mTextureCoords[0][vert_index].x;
        mesh.vertices[vert_index].v0 = p_scene->mMeshes[0]->mTextureCoords[0][vert_index].y;
      }

      if (p_scene->mMeshes[0]->HasTextureCoords(1)) { // 2nd UV coords
        mesh.vertices[vert_index].u0 = p_scene->mMeshes[0]->mTextureCoords[1][vert_index].x;
        mesh.vertices[vert_index].v0 = p_scene->mMeshes[0]->mTextureCoords[1][vert_index].y;
      }
      // TODO: add additional texture coordinates (mTextureCoords[1], etc.)

      // 4Bones: ids and weights
      mesh.vertices[vert_index].bone_id[0] = mesh.vertex_bone_data[vert_index].ids[0];
      mesh.vertices[vert_index].bone_id[1] = mesh.vertex_bone_data[vert_index].ids[1];
      mesh.vertices[vert_index].bone_id[2] = mesh.vertex_bone_data[vert_index].ids[2];
      mesh.vertices[vert_index].bone_id[3] = mesh.vertex_bone_data[vert_index].ids[3];

      mesh.vertices[vert_index].bone_weights[0] = mesh.vertex_bone_data[vert_index].weights[0];
      mesh.vertices[vert_index].bone_weights[1] = mesh.vertex_bone_data[vert_index].weights[1];
      mesh.vertices[vert_index].bone_weights[2] = mesh.vertex_bone_data[vert_index].weights[2];
      mesh.vertices[vert_index].bone_weights[3] = mesh.vertex_bone_data[vert_index].weights[3];
    }

    mesh.indices.resize(number_of_indices);

    i32 numTriangles = p_scene->mMeshes[0]->mNumFaces;
    i32 triIndex = 0;    // Steps through the triangles.
    i32 index_index = 0; // Setps through the indices (index buffer)

    for (; triIndex != number_of_triangles; triIndex++, index_index += 3) { // Note, every 1 triangle = 3 index steps
      aiFace *p_ai_face = &(p_scene->mMeshes[0]->mFaces[triIndex]);

      mesh.indices[index_index + 0] // Offset by 0 (zero)
          = p_ai_face->mIndices[0]; // vertex 0

      mesh.indices[index_index + 1] // Offset by 1
          = p_ai_face->mIndices[1]; // vertex 1

      mesh.indices[index_index + 2] // Offset by 2
          = p_ai_face->mIndices[2]; // vertex 1
    }                               // for ( ; triIndex != numVertices;

    vec3 min_xyz, max_xyz, max_extent_xyz = vec3(0);
    float max_extent = 0.0;
    float scale_for_unit_bbox = 1.0;
    {
      // Assume 1st vertex is both max and min
      min_xyz.x = mesh.vertices[0].x;
      min_xyz.y = mesh.vertices[0].y;
      min_xyz.z = mesh.vertices[0].z;
      max_xyz.x = mesh.vertices[0].x;
      max_xyz.y = mesh.vertices[0].y;
      max_xyz.z = mesh.vertices[0].z;

      for (int index = 0; index != number_of_vertices; index++) {
        if (mesh.vertices[index].x < min_xyz.x) {
          min_xyz.x = mesh.vertices[index].x;
        }
        if (mesh.vertices[index].x > max_xyz.x) {
          max_xyz.x = mesh.vertices[index].x;
        }
        // y...
        if (mesh.vertices[index].y < min_xyz.y) {
          min_xyz.y = mesh.vertices[index].y;
        }
        if (mesh.vertices[index].y > max_xyz.y) {
          max_xyz.y = mesh.vertices[index].y;
        }
        // z...
        if (mesh.vertices[index].z < min_xyz.z) {
          min_xyz.z = mesh.vertices[index].z;
        }
        if (mesh.vertices[index].z > max_xyz.z) {
          max_xyz.z = mesh.vertices[index].z;
        }
      }

      max_extent_xyz.x = max_xyz.x - min_xyz.x;
      max_extent_xyz.y = max_xyz.y - min_xyz.y;
      max_extent_xyz.z = max_xyz.z - min_xyz.z;

      max_extent = max_extent_xyz.x;

      if (max_extent < max_extent_xyz.y) { // Y is bigger
        max_extent = max_extent_xyz.y;
      }

      if (max_extent < max_extent_xyz.z) { // Z is bigger
        max_extent = max_extent_xyz.z;
      }
      scale_for_unit_bbox = 1.0f / max_extent;
    }

    mesh.bounds.max = max_xyz;
    mesh.bounds.min = min_xyz;

    mesh.state.default_animation.name = default_anim;
    mesh.state.active_animation.name = default_anim;
    mesh.state.temp.current_animation = default_anim;
  }
  mesh.pScene = (void *)p_scene;
}
