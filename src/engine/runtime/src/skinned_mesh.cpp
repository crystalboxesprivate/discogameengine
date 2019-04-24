#include <runtime/skinned_mesh.h>
using namespace runtime;

void SkinnedMesh::serialize(Archive &archive) {
  Asset::serialize(archive);

  // archive << lod_info;
  // archive << bulk_data;
}
