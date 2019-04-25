#include <asset/skinned_mesh_factory.h>
#include <assimp/cimport.h>
#include <assimp/mesh.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>

#include <config/config.h>

#include <rapidjson/document.h>
#include <utils/fs.h>
#include <mutex>

#include <utils/string.h>
using utils::string::hash_code;

using runtime::SkinnedMesh;
using namespace glm;

using runtime::SkinnedMeshVertex;

mat4 ai_matrix_to_gl_matrix(const aiMatrix4x4 &mat) {
  return mat4(mat.a1, mat.b1, mat.c1, mat.d1, mat.a2, mat.b2, mat.c2, mat.d2, mat.a3, mat.b3, mat.c3, mat.d3, mat.a4,
              mat.b4, mat.c4, mat.d4);
}

void add_bone_data(VertexBoneData &bone_data, u32 bone_id, float weight) {
  for (i32 Index = 0; Index < sizeof(bone_data.ids) / sizeof(bone_data.ids[0]); Index++) {
    if (bone_data.weights[Index] == 0.0f) {
      bone_data.ids[Index] = (float)bone_id;
      bone_data.weights[Index] = weight;
      return;
    }
  }
}

void SkinnedMeshFactory::load_bones(aiMesh *Mesh, Vector<VertexBoneData> &vertexBoneData,
                                    HashMap<u64, u32> &bone_id_to_index, runtime::SkinnedMesh &mesh_sm) {
  for (i32 boneIndex = 0; boneIndex != Mesh->mNumBones; boneIndex++) {
    i32 bone_idx = 0;
    String bone_name(Mesh->mBones[boneIndex]->mName.data);
    u64 hash = hash_code(bone_name);

    auto it = bone_id_to_index.find(hash);
    if (it == bone_id_to_index.end()) {
      bone_idx = mesh_sm.number_of_bones;
      mesh_sm.number_of_bones++;

      mesh_sm.bone_offsets.push_back(ai_matrix_to_gl_matrix(Mesh->mBones[boneIndex]->mOffsetMatrix));
      bone_id_to_index[hash] = bone_idx;

    } else {
      bone_idx = it->second;
    }

    for (i32 WeightIndex = 0; WeightIndex != Mesh->mBones[boneIndex]->mNumWeights; WeightIndex++) {
      i32 VertexID = Mesh->mBones[boneIndex]->mWeights[WeightIndex].mVertexId;
      float Weight = Mesh->mBones[boneIndex]->mWeights[WeightIndex].mWeight;
      add_bone_data(vertexBoneData[VertexID], bone_idx, Weight);
    }
  }
}

inline String import_path(const String &raw_path) {
  return utils::path::join(config::CONTENT_DIR, raw_path);
}

void make_hierarchy(runtime::animation::Node &node, aiNode *ai_node, HashMap<u64, u32>& bone_id_to_index) {
  node.name = ai_node->mName.C_Str();
  node.bone_index = bone_id_to_index[hash_code(ai_node->mName.C_Str())];

  node.transform = ai_matrix_to_gl_matrix(ai_node->mTransformation);
  node.children.resize(ai_node->mNumChildren);
  for (int x = 0; x < node.children.size(); x++) {
    auto &child = node.children[x];
    make_hierarchy(child, ai_node->mChildren[x], bone_id_to_index);
  }
}

void load_animation(runtime::animation::Animation &animation, aiAnimation *ai_animation) {
  assert(ai_animation);
  auto &ai = *ai_animation;
  animation.name = ai.mName.C_Str();

  animation.duration = ai.mDuration;
  animation.ticks_per_second = ai.mTicksPerSecond;

  animation.duration_seconds = animation.duration / animation.ticks_per_second;
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
                                             runtime::SkinnedMesh &mesh, bool has_exit_time) {
  u64 hash = hash_code(friendly_name);

  i32 flags =
      aiProcess_Triangulate | aiProcess_OptimizeMeshes | aiProcess_OptimizeGraph | aiProcess_JoinIdenticalVertices;

  const aiScene *ai_scene = aiImportFile(filename.c_str(), flags);

  if (!ai_scene)
    return false;

  mesh.animations.push_back(runtime::animation::Animation());

  load_animation(mesh.animations.back(), ai_scene->mAnimations[0]);
  mesh.ticks_per_second = mesh.animations.back().ticks_per_second;
  mesh.animations.back().name = friendly_name;

  aiReleaseImport(ai_scene);
  return true;
}
std::mutex m;
void SkinnedMeshFactory::load_asset_data(asset::Asset &asset) {

  SkinnedMesh &mesh = *reinterpret_cast<SkinnedMesh *>(&asset);
  i32 flags = aiProcess_Triangulate | aiProcess_OptimizeMeshes | aiProcess_OptimizeGraph |
              aiProcess_JoinIdenticalVertices | aiProcess_GenNormals | aiProcess_CalcTangentSpace;

  struct AnimationNameFilename {
    String name;
    String filename;
  };

  // mesh.number_of_vertices = 0;
  Vector<VertexBoneData> vertex_bone_data;

  utils::free_vector(vertex_bone_data);

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

  vertex_bone_data.resize(p_scene->mMeshes[0]->mNumVertices);

  HashMap<u64, u32> bone_id_to_index;
  this->load_bones(this->p_scene->mMeshes[0], vertex_bone_data, bone_id_to_index, mesh);
  mesh.default_animation = 3;

  m.lock();
  for (auto &animation : animation_filenames) {
    load_mesh_animation(animation.name, animation.filename, mesh);
  }
  m.unlock();

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
      mesh.vertices[vert_index].bone_id[0] = vertex_bone_data[vert_index].ids[0];
      mesh.vertices[vert_index].bone_id[1] = vertex_bone_data[vert_index].ids[1];
      mesh.vertices[vert_index].bone_id[2] = vertex_bone_data[vert_index].ids[2];
      mesh.vertices[vert_index].bone_id[3] = vertex_bone_data[vert_index].ids[3];

      mesh.vertices[vert_index].bone_weights[0] = vertex_bone_data[vert_index].weights[0];
      mesh.vertices[vert_index].bone_weights[1] = vertex_bone_data[vert_index].weights[1];
      mesh.vertices[vert_index].bone_weights[2] = vertex_bone_data[vert_index].weights[2];
      mesh.vertices[vert_index].bone_weights[3] = vertex_bone_data[vert_index].weights[3];
    }

    mesh.indices.resize(number_of_indices);

    i32 num_triangles = p_scene->mMeshes[0]->mNumFaces;
    i32 tri_index = 0;
    i32 index_index = 0;

    for (; tri_index != number_of_triangles; tri_index++, index_index += 3) {
      aiFace *p_ai_face = &(p_scene->mMeshes[0]->mFaces[tri_index]);

      mesh.indices[index_index + 0] = p_ai_face->mIndices[0];
      mesh.indices[index_index + 1] = p_ai_face->mIndices[1];
      mesh.indices[index_index + 2] = p_ai_face->mIndices[2];
    }

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
  }
  make_hierarchy(mesh.hierarchy, p_scene->mRootNode, bone_id_to_index);

  aiReleaseImport(p_scene);
}
