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

mat4 AIMatrixToGLMMatrix2(const aiMatrix4x4 &mat) {
  return mat4(mat.a1, mat.b1, mat.c1, mat.d1, mat.a2, mat.b2, mat.c2, mat.d2, mat.a3, mat.b3, mat.c3, mat.d3, mat.a4,
              mat.b4, mat.c4, mat.d4);
}

namespace runtime {
void VertexBoneData::AddBoneData(unsigned int BoneID, float Weight) {
  for (unsigned int Index = 0; Index < sizeof(this->ids) / sizeof(this->ids[0]); Index++) {
    if (this->weights[Index] == 0.0f) {
      this->ids[Index] = (float)BoneID;
      this->weights[Index] = Weight;
			return;
    }
  }
}
} // namespace runtime
void SkinnedMeshFactory::LoadBones(aiMesh *Mesh, Vector<runtime::VertexBoneData> &vertexBoneData, runtime::SkinnedMesh& mesh_sm) {
  for (unsigned int boneIndex = 0; boneIndex != Mesh->mNumBones; boneIndex++) {
    unsigned int BoneIndex = 0;
    String BoneName(Mesh->mBones[boneIndex]->mName.data);
    //	Map<String /*BoneName*/, unsigned int /*BoneIndex*/> mMapping;
    // 	std::vector<sBoneInfo> mInfo;

    Map<String, unsigned int>::iterator it = mesh_sm.bone_name_to_bone_index.find(BoneName);
    if (it == mesh_sm.bone_name_to_bone_index.end()) {
      BoneIndex = mesh_sm.number_of_bones;
      mesh_sm.number_of_bones++;
      runtime::sBoneInfo bi;
      mesh_sm.bone_info.push_back(bi);

      mesh_sm.bone_info[BoneIndex].BoneOffset = AIMatrixToGLMMatrix2(Mesh->mBones[boneIndex]->mOffsetMatrix);
      mesh_sm.bone_name_to_bone_index[BoneName] = BoneIndex;
    } else {
      BoneIndex = it->second;
    }

    for (unsigned int WeightIndex = 0; WeightIndex != Mesh->mBones[boneIndex]->mNumWeights; WeightIndex++) {
      unsigned int VertexID =
          /*mMeshEntries[MeshIndex].BaseVertex +*/ Mesh->mBones[boneIndex]->mWeights[WeightIndex].mVertexId;
      float Weight = Mesh->mBones[boneIndex]->mWeights[WeightIndex].mWeight;
      vertexBoneData[VertexID].AddBoneData(BoneIndex, Weight);
    }
  }
}

inline String import_path(const String &raw_path) {
  return utils::path::join(config::CONTENT_DIR, raw_path);
}

bool SkinnedMeshFactory::LoadMeshAnimation(const String &friendly_name, const String &filename,
                                           runtime::SkinnedMesh &mesh, bool has_exit_time) // Only want animations
{
  Map<String, AnimationInfo>::iterator it_animation =
      mesh.animation_name_to_pscene.find(friendly_name);

  if (it_animation != mesh.animation_name_to_pscene.end()) {
    return false;
  }

  unsigned int flags =
      aiProcess_Triangulate | aiProcess_OptimizeMeshes | aiProcess_OptimizeGraph | aiProcess_JoinIdenticalVertices;

  Assimp::Importer *importer = new Assimp::Importer();
  AnimationInfo anim_info;
  anim_info.friendly_name = friendly_name;
  anim_info.filename = filename;
  const aiScene *ai_scene = importer->ReadFile(anim_info.filename.c_str(), flags);
  anim_info.has_exit_time = has_exit_time;

  if (!ai_scene)
    return false;

  anim_info.ai_animation = ai_scene->mAnimations[0];
  // Get duration is seconds
  anim_info.duration = (float)(anim_info.ai_animation->mDuration / anim_info.ai_animation->mTicksPerSecond);
  mesh.animation_name_to_pscene[anim_info.friendly_name] = anim_info;
  return true;
}

void SkinnedMeshFactory::load_asset_data(asset::Asset &asset) {
  SkinnedMesh &mesh = *reinterpret_cast<SkinnedMesh *>(&asset);
  unsigned int Flags = aiProcess_Triangulate | aiProcess_OptimizeMeshes | aiProcess_OptimizeGraph |
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
  // Assimp::Importer mImporter;
  // pScene = mImporter.ReadFile(mesh_filename.c_str(), Flags);
  pScene = aiImportFile(mesh_filename.c_str(), Flags);

  assert(pScene);

  mesh.global_inverse_transformation = AIMatrixToGLMMatrix2(pScene->mRootNode->mTransformation);
  mesh.global_inverse_transformation = inverse(mesh.global_inverse_transformation);

  mesh.number_of_vertices = pScene->mMeshes[0]->mNumVertices;
  // This is the vertex information for JUST the bone stuff
  mesh.vertex_bone_data.resize(mesh.number_of_vertices);

  this->LoadBones(this->pScene->mMeshes[0], mesh.vertex_bone_data, mesh);
  String default_anim = "runForward";

  for (auto &animation : animation_filenames) {
    if (default_anim == "")
      default_anim = animation.name;
    LoadMeshAnimation(animation.name, animation.filename, mesh);
  }

  // Vector<sVertex_xyz_rgba_n_uv2_bt_4Bones> mesh.vertices;
  // Vector<i32> mesh.indices;
  {

    // sModelDrawInfo theSMDrawInfo;

    // theSMDrawInfo.mesh_filename = filename;
    // theSMDrawInfo.friendlyName = friendlyName;

    // Copy the data from assimp format into the sModelDrawInfo format...
    i32 numberOfVertices = pScene->mMeshes[0]->mNumVertices;
    i32 numberOfTriangles = pScene->mMeshes[0]->mNumFaces;

    // We used the "triangulate" option when loading so all the primitives
    //	will be triangles, but BEWARE!
    i32 numberOfIndices = numberOfTriangles * 3;

    // Allocate the vertex array (it's a c-style array)
    // theSMDrawInfo.pMeshData = new cMesh();

    mesh.vertices.resize(numberOfVertices);

    // Danger Will Robinson!
    // You don't really need to do this, but at least it will clear it to zero.
    // (compiler will complain that it's 'not safe', etc.)

    // memset(mesh.vertices.data(), 0,
    //        sizeof(sVertex_xyz_rgba_n_uv2_bt_4Bones) * numberOfVertices);

    for (unsigned int vertIndex = 0; vertIndex != numberOfVertices; vertIndex++) {
      mesh.vertices[vertIndex].x = pScene->mMeshes[0]->mVertices[vertIndex].x;
      mesh.vertices[vertIndex].y = pScene->mMeshes[0]->mVertices[vertIndex].y;
      mesh.vertices[vertIndex].z = pScene->mMeshes[0]->mVertices[vertIndex].z;

      // Normals...
      mesh.vertices[vertIndex].nx = pScene->mMeshes[0]->mNormals[vertIndex].x;
      mesh.vertices[vertIndex].ny = pScene->mMeshes[0]->mNormals[vertIndex].y;
      mesh.vertices[vertIndex].nz = pScene->mMeshes[0]->mNormals[vertIndex].z;

      //// Colours...
      //// (If there are no colours, make it hit pink)
      //// Note: the method is because you could have more than one set of
      ////	vertex colours in the model (Why? Who the heck knows?)
      // if (pScene->mMeshes[0]->HasVertexColors(0)) {
      //  mesh.vertices[vertIndex].r = pScene->mMeshes[0]->mColors[vertIndex]->r;
      //  mesh.vertices[vertIndex].g = pScene->mMeshes[0]->mColors[vertIndex]->g;
      //  mesh.vertices[vertIndex].b = pScene->mMeshes[0]->mColors[vertIndex]->b;
      //  mesh.vertices[vertIndex].a = pScene->mMeshes[0]->mColors[vertIndex]->a;
      //} else { // hotpink	#FF69B4	rgb(255,105,180)
      //  mesh.vertices[vertIndex].r = 1.f;
      //  mesh.vertices[vertIndex].g = 105.f / 255.f;
      //  mesh.vertices[vertIndex].b = 180.f / 255.f;
      //  mesh.vertices[vertIndex].a = 1.f;
      //}

      // bi-normal  (or bi-tangent)
      mesh.vertices[vertIndex].bx = pScene->mMeshes[0]->mBitangents[vertIndex].x;
      mesh.vertices[vertIndex].by = pScene->mMeshes[0]->mBitangents[vertIndex].y;
      mesh.vertices[vertIndex].bz = pScene->mMeshes[0]->mBitangents[vertIndex].z;

      // Tangent
      mesh.vertices[vertIndex].tx = pScene->mMeshes[0]->mTangents[vertIndex].x;
      mesh.vertices[vertIndex].ty = pScene->mMeshes[0]->mTangents[vertIndex].y;
      mesh.vertices[vertIndex].tz = pScene->mMeshes[0]->mTangents[vertIndex].z;

      // uv2 (which are odd in assimp)
      // Note that there is an array of texture coordinates,
      // up to 8 (actually). Usually, there's only 1
      if (pScene->mMeshes[0]->HasTextureCoords(0)) // 1st UV coords
      {
        // Assume there's 1... (at least)
        mesh.vertices[vertIndex].u0 = pScene->mMeshes[0]->mTextureCoords[0][vertIndex].x;
        mesh.vertices[vertIndex].v0 = pScene->mMeshes[0]->mTextureCoords[0][vertIndex].y;
      }
      if (pScene->mMeshes[0]->HasTextureCoords(1)) // 2nd UV coords
      {
        mesh.vertices[vertIndex].u0 = pScene->mMeshes[0]->mTextureCoords[1][vertIndex].x;
        mesh.vertices[vertIndex].v0 = pScene->mMeshes[0]->mTextureCoords[1][vertIndex].y;
      }
      // TODO: add additional texture coordinates (mTextureCoords[1], etc.)

      // 4Bones: ids and weights
      mesh.vertices[vertIndex].bone_id[0] = mesh.vertex_bone_data[vertIndex].ids[0];
      mesh.vertices[vertIndex].bone_id[1] = mesh.vertex_bone_data[vertIndex].ids[1];
      mesh.vertices[vertIndex].bone_id[2] = mesh.vertex_bone_data[vertIndex].ids[2];
      mesh.vertices[vertIndex].bone_id[3] = mesh.vertex_bone_data[vertIndex].ids[3];

      mesh.vertices[vertIndex].bone_weights[0] = mesh.vertex_bone_data[vertIndex].weights[0];
      mesh.vertices[vertIndex].bone_weights[1] = mesh.vertex_bone_data[vertIndex].weights[1];
      mesh.vertices[vertIndex].bone_weights[2] = mesh.vertex_bone_data[vertIndex].weights[2];
      mesh.vertices[vertIndex].bone_weights[3] = mesh.vertex_bone_data[vertIndex].weights[3];

    } // for ( unsigned int vertIndex = 0;

    // And the triangles

    // Allocate the array to hold the indices (triangle) info

    // Allocate the array for that (indices NOT triangles)
    mesh.indices.resize(numberOfIndices);

    // Danger Will Robinson!
    // You don't really need to do this, but at least it will clear it to zero.
    // (compiler will complain that it's 'not safe', etc.)
    // memset(mesh.indices, 0, sizeof(unsigned int) * theSMDrawInfo.numberOfIndices);

    unsigned int numTriangles = pScene->mMeshes[0]->mNumFaces;
    unsigned int triIndex = 0;                                         // Steps through the triangles.
    unsigned int indexIndex = 0;                                       // Setps through the indices (index buffer)
    for (; triIndex != numberOfTriangles; triIndex++, indexIndex += 3) // Note, every 1 triangle = 3 index steps
    {
      // Get the triangle at this triangle index...
      aiFace *pAIFace = &(pScene->mMeshes[0]->mFaces[triIndex]);

      mesh.indices[indexIndex + 0] // Offset by 0 (zero)
          = pAIFace->mIndices[0];  // vertex 0

      mesh.indices[indexIndex + 1] // Offset by 1
          = pAIFace->mIndices[1];  // vertex 1

      mesh.indices[indexIndex + 2] // Offset by 2
          = pAIFace->mIndices[2];  // vertex 1
    }                              // for ( ; triIndex != numVertices;

    // Calculate the extents on the mesh
    // (Note, because I'm a bone head, this is dupicated...)
    // name = theSMDrawInfo.friendlyName;
    // numberOfIndices = theSMDrawInfo.numberOfIndices;
    // numberOfTriangles = theSMDrawInfo.numberOfTriangles;
    // numberOfVertices = theSMDrawInfo.numberOfVertices;
    // CalculateExtents();
    vec3 minXYZ, maxXYZ, maxExtentXYZ = vec3(0);
    float maxExtent = 0.0;
    float scaleForUnitBBox = 1.0;
    {
      // Assume 1st vertex is both max and min
      minXYZ.x = mesh.vertices[0].x;
      minXYZ.y = mesh.vertices[0].y;
      minXYZ.z = mesh.vertices[0].z;
      maxXYZ.x = mesh.vertices[0].x;
      maxXYZ.y = mesh.vertices[0].y;
      maxXYZ.z = mesh.vertices[0].z;

      for (int index = 0; index != numberOfVertices; index++) {
        if (mesh.vertices[index].x < minXYZ.x) {
          minXYZ.x = mesh.vertices[index].x;
        }
        if (mesh.vertices[index].x > maxXYZ.x) {
          maxXYZ.x = mesh.vertices[index].x;
        }
        // y...
        if (mesh.vertices[index].y < minXYZ.y) {
          minXYZ.y = mesh.vertices[index].y;
        }
        if (mesh.vertices[index].y > maxXYZ.y) {
          maxXYZ.y = mesh.vertices[index].y;
        }
        // z...
        if (mesh.vertices[index].z < minXYZ.z) {
          minXYZ.z = mesh.vertices[index].z;
        }
        if (mesh.vertices[index].z > maxXYZ.z) {
          maxXYZ.z = mesh.vertices[index].z;
        }

      } // for ( int index...

      maxExtentXYZ.x = maxXYZ.x - minXYZ.x;
      maxExtentXYZ.y = maxXYZ.y - minXYZ.y;
      maxExtentXYZ.z = maxXYZ.z - minXYZ.z;

      maxExtent = maxExtentXYZ.x;
      if (maxExtent < maxExtentXYZ.y) { // Y is bigger
        maxExtent = maxExtentXYZ.y;
      }
      if (maxExtent < maxExtentXYZ.z) { // Z is bigger
        maxExtent = maxExtentXYZ.z;
      }
      //
      scaleForUnitBBox = 1.0f / maxExtent;
    }

    mesh.bounds.max = maxXYZ;
    mesh.bounds.min = minXYZ;

    mesh.state.default_animation.name = default_anim;
    mesh.state.active_animation.name = default_anim;
    mesh.state.temp.current_animation = default_anim;
  }
  mesh.pScene = (void *)pScene;
  // aiReleaseImport(pScene);
}
