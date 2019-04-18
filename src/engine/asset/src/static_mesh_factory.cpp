#include <asset/static_mesh_factory.h>
#include <assimp/cimport.h>
#include <assimp/mesh.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

using runtime::StaticMesh;

void StaticMeshFactory::load_asset_data(asset::Asset &asset) {
  StaticMesh &mesh = *reinterpret_cast<StaticMesh *>(&asset);
  mesh.bulk_data.free();
  unsigned int flags = aiProcess_Triangulate | aiProcess_OptimizeMeshes | aiProcess_OptimizeGraph |
                       aiProcess_JoinIdenticalVertices | aiProcess_GenNormals | aiProcess_CalcTangentSpace;
  const aiScene *scene = aiImportFile(mesh.source_filename.c_str(), flags);
  assert(scene);
  assert(scene->mNumMeshes);
  aiMesh *ai_mesh_ptr = scene->mMeshes[0];
  assert(ai_mesh_ptr);
  auto &ai_mesh = *ai_mesh_ptr;
  assert(ai_mesh.HasFaces());
  assert(ai_mesh.HasNormals());
  assert(ai_mesh.HasPositions());
  // assert(ai_mesh.HasTangentsAndBitangents());

  if (!ai_mesh.HasTextureCoords(0))
    DEBUG_LOG(Assets, Error, "Mesh %s doesn't contain texture coordinates", mesh.source_filename.c_str());

  StaticMesh::LodInfo lod;
  lod.index_count = ai_mesh.mNumFaces * 3;
  lod.vertex_count = ai_mesh.mNumVertices;
  lod.has_colors = false;

  usize buffer_size;
  {
    usize index_byte_count = lod.index_count * sizeof(i32);
    usize positions_byte_count = lod.vertex_count * sizeof(float) * 3;
    usize texcoords_byte_count = lod.vertex_count * sizeof(float) * 2;
    usize normals_byte_count = lod.vertex_count * sizeof(float) * 3;
    usize tangents_byte_count = normals_byte_count;
    usize colors_byte_count = lod.has_colors ? lod.vertex_count * sizeof(float) * 3 : 0;
    buffer_size = index_byte_count + positions_byte_count + texcoords_byte_count + normals_byte_count +
                  tangents_byte_count + colors_byte_count;
  }

  mesh.bulk_data.allocate(buffer_size);

  mesh.bounds.min.x = ai_mesh.mVertices[0].x;
  mesh.bounds.min.y = ai_mesh.mVertices[0].y;
  mesh.bounds.min.z = ai_mesh.mVertices[0].z;
  mesh.bounds.max = mesh.bounds.min;

  //#if 0
  u32 offset = 0;
  u32 stride = 0;
  for (u32 x = 0; x < ai_mesh.mNumFaces; x++) {
    const struct aiFace &face = ai_mesh.mFaces[x];
    // Support only triangles
    assert(face.mNumIndices == 3);
    u32 face_arr[3] = {face.mIndices[0], face.mIndices[1], face.mIndices[2]};
    stride = 4 * 3;
    memcpy(mesh.bulk_data.get_data() + offset, face_arr, stride);
    offset += stride;
  }
  // todo optimize
  for (u32 x = 0; x < ai_mesh.mNumVertices; x++) {
    auto vertex = ai_mesh.mVertices[x];
    {
      if (vertex.x < mesh.bounds.min.x) {
        mesh.bounds.min.x = vertex.x;
      }
      if (vertex.y < mesh.bounds.min.y) {
        mesh.bounds.min.y = vertex.y;
      }
      if (vertex.z < mesh.bounds.min.z) {
        mesh.bounds.min.z = vertex.z;
      }

      if (vertex.x > mesh.bounds.max.x) {
        mesh.bounds.max.x = vertex.x;
      }
      if (vertex.y > mesh.bounds.max.y) {
        mesh.bounds.max.y = vertex.y;
      }
      if (vertex.z > mesh.bounds.max.z) {
        mesh.bounds.max.z = vertex.z;
      }
    }

    stride = 4 * 3;
    memcpy(mesh.bulk_data.get_data() + offset, &vertex.x, stride);
    offset += stride;
  }

  for (u32 x = 0; x < ai_mesh.mNumVertices; x++) {
    stride = 4 * 2;
    if (ai_mesh.HasTextureCoords(0)) {
      auto v = ai_mesh.mTextureCoords[0][x];
      memcpy(mesh.bulk_data.get_data() + offset, &v.x, stride);
    }
    offset += stride;
  }

  for (u32 x = 0; x < ai_mesh.mNumVertices; x++) {
    auto v = ai_mesh.mNormals[x];
    stride = 4 * 3;
    memcpy(mesh.bulk_data.get_data() + offset, &v.x, stride);
    offset += stride;
  }

  for (u32 x = 0; x < ai_mesh.mNumVertices; x++) {
    stride = 4 * 3;
    if (ai_mesh.HasTangentsAndBitangents()) {
      auto v = ai_mesh.mTangents[x];
      memcpy(mesh.bulk_data.get_data() + offset, &v.x, stride);
    }
    offset += stride;
  }

  if (lod.has_colors) {
    for (u32 x = 0; x < ai_mesh.mNumVertices; x++) {
      auto v = ai_mesh.mColors[0][x];
      stride = 4 * 3;
      memcpy(mesh.bulk_data.get_data() + offset, &v.r, stride);
      offset += stride;
    }
  }
  //#endif
  assert(!mesh.lod_info.size());
  mesh.lod_info.push_back(lod);
  aiReleaseImport(scene);
}
