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

mat4 AIMatrixToGLMMatrix(const aiMatrix4x4 &mat) {
  return mat4(mat.a1, mat.b1, mat.c1, mat.d1, mat.a2, mat.b2, mat.c2, mat.d2, mat.a3, mat.b3, mat.c3, mat.d3, mat.a4,
              mat.b4, mat.c4, mat.d4);
}

static const i32 NUMBEROFBONES = 4;
struct sVertex_xyz_rgba_n_uv2_bt_4Bones {
  sVertex_xyz_rgba_n_uv2_bt_4Bones()
      : x(0.0f)
      , y(0.0f)
      , z(0.0f)
      , w(1.0f)
      , r(0.0f)
      , g(0.0f)
      , b(0.0f)
      , a(1.0f)
      , // Note alpha is 1.0
      nx(0.0f)
      , ny(0.0f)
      , nz(0.0f)
      , nw(1.0f)
      , u0(0.0f)
      , v0(0.0f)
      , u1(0.0f)
      , v1(0.0f)
      , tx(0.0f)
      , ty(0.0f)
      , tz(0.0f)
      , tw(1.0f)
      , bx(0.0f)
      , by(0.0f)
      , bz(0.0f)
      , bw(1.0f) {
    //#ifdef _DEBUG
    memset(this->boneID, 0, sizeof(u32) * NUMBEROFBONES);
    memset(this->boneWeights, 0, sizeof(float) * NUMBEROFBONES);
    // So these are essentially this:
    //		unsigned int boneID[4];
    //		float boneWeights[4];
    //#endif // DEBUG
  };
  // Destructor isn't really needed here
  //~sVertex_xyz_rgba_n_uv2_bt_skinnedMesh();

  float x, y, z, w;     // 16
  float r, g, b, a;     // 32
  float nx, ny, nz, nw; // 48
  float u0, v0, u1, v1; // 60
  float tx, ty, tz, tw; // tangent				//
  float bx, by, bz, bw; // bi-normal			//
  // For the 4 bone skinned mesh information
  float boneID[NUMBEROFBONES];      // New		//
  float boneWeights[NUMBEROFBONES]; // New		//
};

void VertexBoneData::AddBoneData(unsigned int BoneID, float Weight) {
  for (unsigned int Index = 0; Index < sizeof(this->ids) / sizeof(this->ids[0]); Index++) {
    if (this->weights[Index] == 0.0f) {
      this->ids[Index] = (float)BoneID;
      this->weights[Index] = Weight;
    }
  }
}

void SkinnedMeshFactory::LoadBones(aiMesh *Mesh, Vector<VertexBoneData> &vertexBoneData) {
  for (unsigned int boneIndex = 0; boneIndex != Mesh->mNumBones; boneIndex++) {
    unsigned int BoneIndex = 0;
    std::string BoneName(Mesh->mBones[boneIndex]->mName.data);
    //	std::map<std::string /*BoneName*/, unsigned int /*BoneIndex*/> mMapping;
    // 	std::vector<sBoneInfo> mInfo;

    std::map<std::string, unsigned int>::iterator it = this->m_mapBoneNameToBoneIndex.find(BoneName);
    if (it == this->m_mapBoneNameToBoneIndex.end()) {
      BoneIndex = this->mNumBones;
      this->mNumBones++;
      sBoneInfo bi;
      this->mBoneInfo.push_back(bi);

      this->mBoneInfo[BoneIndex].BoneOffset = AIMatrixToGLMMatrix(Mesh->mBones[boneIndex]->mOffsetMatrix);
      this->m_mapBoneNameToBoneIndex[BoneName] = BoneIndex;
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

bool SkinnedMeshFactory::LoadMeshAnimation(const std::string &friendlyName, const std::string &filename,
                                           bool hasExitTime) // Only want animations
{
  // Already loaded this?
  std::map<std::string /*animation FRIENDLY name*/, sAnimationInfo>::iterator itAnimation =
      this->mapAnimationFriendlyNameTo_pScene.find(friendlyName);

  // Found it?
  if (itAnimation != this->mapAnimationFriendlyNameTo_pScene.end()) { // Yup. So we already loaded it.
    return false;
  }

  //	std::map< std::string /*animationfile*/,
  //		      const aiScene* > mapAnimationNameTo_pScene;		// Animations

  unsigned int Flags =
      aiProcess_Triangulate | aiProcess_OptimizeMeshes | aiProcess_OptimizeGraph | aiProcess_JoinIdenticalVertices;

  Assimp::Importer *pImporter = new Assimp::Importer();
  sAnimationInfo animInfo;
  animInfo.friendlyName = friendlyName;
  animInfo.fileName = filename;
  animInfo.pAIScene = pImporter->ReadFile(animInfo.fileName.c_str(), Flags);
  animInfo.bHasExitTime = hasExitTime;

  // Get duration is seconds
  animInfo.duration =
      (float)(animInfo.pAIScene->mAnimations[0]->mDuration / animInfo.pAIScene->mAnimations[0]->mTicksPerSecond);

  if (!animInfo.pAIScene) {
    return false;
  }
  
  // Animation scene is loaded (we assume)
  // Add it to the map
  // this->mapAnimationNameTo_pScene[filename] = animInfo;
  this->mapAnimationFriendlyNameTo_pScene[animInfo.friendlyName] = animInfo;

  return true;
}

void SkinnedMeshFactory::load_asset_data(asset::Asset &asset) {
  unsigned int Flags = aiProcess_Triangulate | aiProcess_OptimizeMeshes | aiProcess_OptimizeGraph |
                       aiProcess_JoinIdenticalVertices | aiProcess_GenNormals | aiProcess_CalcTangentSpace;
  struct AnimationNameFilename {
    String name;
    String filename;
  };

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
  //Assimp::Importer mImporter;
  //pScene = mImporter.ReadFile(mesh_filename.c_str(), Flags);
  pScene = aiImportFile(mesh_filename.c_str(), Flags);


  assert(pScene);

  mGlobalInverseTransformation = AIMatrixToGLMMatrix(pScene->mRootNode->mTransformation);
  mGlobalInverseTransformation = inverse(mGlobalInverseTransformation);

  m_numberOfVertices = pScene->mMeshes[0]->mNumVertices;
  // This is the vertex information for JUST the bone stuff
  vecVertexBoneData.resize(m_numberOfVertices);

  this->LoadBones(this->pScene->mMeshes[0], this->vecVertexBoneData);

  for (auto &animation : animation_filenames) {
    LoadMeshAnimation(animation.name, animation.filename);
  }

  Vector<sVertex_xyz_rgba_n_uv2_bt_4Bones> pVertices;
  Vector<i32> pIndices;
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

    pVertices.resize(numberOfVertices);

    // Danger Will Robinson!
    // You don't really need to do this, but at least it will clear it to zero.
    // (compiler will complain that it's 'not safe', etc.)

    // memset(pVertices.data(), 0,
    //        sizeof(sVertex_xyz_rgba_n_uv2_bt_4Bones) * numberOfVertices);

    for (unsigned int vertIndex = 0; vertIndex != numberOfVertices; vertIndex++) {
      pVertices[vertIndex].x = pScene->mMeshes[0]->mVertices[vertIndex].x;
      pVertices[vertIndex].y = pScene->mMeshes[0]->mVertices[vertIndex].y;
      pVertices[vertIndex].z = pScene->mMeshes[0]->mVertices[vertIndex].z;

      // Normals...
      pVertices[vertIndex].nx = pScene->mMeshes[0]->mNormals[vertIndex].x;
      pVertices[vertIndex].ny = pScene->mMeshes[0]->mNormals[vertIndex].y;
      pVertices[vertIndex].nz = pScene->mMeshes[0]->mNormals[vertIndex].z;

      // Colours...
      // (If there are no colours, make it hit pink)
      // Note: the method is because you could have more than one set of
      //	vertex colours in the model (Why? Who the heck knows?)
      if (pScene->mMeshes[0]->HasVertexColors(0)) {
        pVertices[vertIndex].r = pScene->mMeshes[0]->mColors[vertIndex]->r;
        pVertices[vertIndex].g = pScene->mMeshes[0]->mColors[vertIndex]->g;
        pVertices[vertIndex].b = pScene->mMeshes[0]->mColors[vertIndex]->b;
        pVertices[vertIndex].a = pScene->mMeshes[0]->mColors[vertIndex]->a;
      } else { // hotpink	#FF69B4	rgb(255,105,180)
        pVertices[vertIndex].r = 1.f;
        pVertices[vertIndex].g = 105.f / 255.f;
        pVertices[vertIndex].b = 180.f / 255.f;
        pVertices[vertIndex].a = 1.f;
      }

      // bi-normal  (or bi-tangent)
      pVertices[vertIndex].bx = pScene->mMeshes[0]->mBitangents[vertIndex].x;
      pVertices[vertIndex].by = pScene->mMeshes[0]->mBitangents[vertIndex].y;
      pVertices[vertIndex].bz = pScene->mMeshes[0]->mBitangents[vertIndex].z;

      // Tangent
      pVertices[vertIndex].tx = pScene->mMeshes[0]->mTangents[vertIndex].x;
      pVertices[vertIndex].ty = pScene->mMeshes[0]->mTangents[vertIndex].y;
      pVertices[vertIndex].tz = pScene->mMeshes[0]->mTangents[vertIndex].z;

      // uv2 (which are odd in assimp)
      // Note that there is an array of texture coordinates,
      // up to 8 (actually). Usually, there's only 1
      if (pScene->mMeshes[0]->HasTextureCoords(0)) // 1st UV coords
      {
        // Assume there's 1... (at least)
        pVertices[vertIndex].u0 = pScene->mMeshes[0]->mTextureCoords[0][vertIndex].x;
        pVertices[vertIndex].v0 = pScene->mMeshes[0]->mTextureCoords[0][vertIndex].y;
      }
      if (pScene->mMeshes[0]->HasTextureCoords(1)) // 2nd UV coords
      {
        pVertices[vertIndex].u0 = pScene->mMeshes[0]->mTextureCoords[1][vertIndex].x;
        pVertices[vertIndex].v0 = pScene->mMeshes[0]->mTextureCoords[1][vertIndex].y;
      }
      // TODO: add additional texture coordinates (mTextureCoords[1], etc.)

      // 4Bones: ids and weights
      pVertices[vertIndex].boneID[0] = vecVertexBoneData[vertIndex].ids[0];
      pVertices[vertIndex].boneID[1] = vecVertexBoneData[vertIndex].ids[1];
      pVertices[vertIndex].boneID[2] = vecVertexBoneData[vertIndex].ids[2];
      pVertices[vertIndex].boneID[3] = vecVertexBoneData[vertIndex].ids[3];

      pVertices[vertIndex].boneWeights[0] = vecVertexBoneData[vertIndex].weights[0];
      pVertices[vertIndex].boneWeights[1] = vecVertexBoneData[vertIndex].weights[1];
      pVertices[vertIndex].boneWeights[2] = vecVertexBoneData[vertIndex].weights[2];
      pVertices[vertIndex].boneWeights[3] = vecVertexBoneData[vertIndex].weights[3];

    } // for ( unsigned int vertIndex = 0;

    // And the triangles

    // Allocate the array to hold the indices (triangle) info

    // Allocate the array for that (indices NOT triangles)
    pIndices.resize(numberOfIndices);

    // Danger Will Robinson!
    // You don't really need to do this, but at least it will clear it to zero.
    // (compiler will complain that it's 'not safe', etc.)
    // memset(pIndices, 0, sizeof(unsigned int) * theSMDrawInfo.numberOfIndices);

    unsigned int numTriangles = pScene->mMeshes[0]->mNumFaces;
    unsigned int triIndex = 0;                                         // Steps through the triangles.
    unsigned int indexIndex = 0;                                       // Setps through the indices (index buffer)
    for (; triIndex != numberOfTriangles; triIndex++, indexIndex += 3) // Note, every 1 triangle = 3 index steps
    {
      // Get the triangle at this triangle index...
      aiFace *pAIFace = &(pScene->mMeshes[0]->mFaces[triIndex]);

      pIndices[indexIndex + 0]    // Offset by 0 (zero)
          = pAIFace->mIndices[0]; // vertex 0

      pIndices[indexIndex + 1]    // Offset by 1
          = pAIFace->mIndices[1]; // vertex 1

      pIndices[indexIndex + 2]    // Offset by 2
          = pAIFace->mIndices[2]; // vertex 1
    }                             // for ( ; triIndex != numVertices;

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
      minXYZ.x = pVertices[0].x;
      minXYZ.y = pVertices[0].y;
      minXYZ.z = pVertices[0].z;
      maxXYZ.x = pVertices[0].x;
      maxXYZ.y = pVertices[0].y;
      maxXYZ.z = pVertices[0].z;

      for (int index = 0; index != numberOfVertices; index++) {
        if (pVertices[index].x < minXYZ.x) {
          minXYZ.x = pVertices[index].x;
        }
        if (pVertices[index].x > maxXYZ.x) {
          maxXYZ.x = pVertices[index].x;
        }
        // y...
        if (pVertices[index].y < minXYZ.y) {
          minXYZ.y = pVertices[index].y;
        }
        if (pVertices[index].y > maxXYZ.y) {
          maxXYZ.y = pVertices[index].y;
        }
        // z...
        if (pVertices[index].z < minXYZ.z) {
          minXYZ.z = pVertices[index].z;
        }
        if (pVertices[index].z > maxXYZ.z) {
          maxXYZ.z = pVertices[index].z;
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
  }

  aiReleaseImport(pScene);
}
