#pragma once

#include <asset/asset.h>
#include <runtime/mesh.h>

namespace renderer {
struct Resource;
}

namespace runtime {
struct SkinnedMeshResource;
struct SkinnedMesh : public asset::Asset {
  declare_asset_type(SkinnedMesh);
  SkinnedMesh() {
  }

  Blob bulk_data;
  virtual void free() override {
    bulk_data.free();
  }

  SkinnedMeshResource *get_render_resource();
  virtual void serialize(Archive &archive) override;

  struct LodInfo {
    usize index_count;
    usize vertex_count;
    bool has_colors;
    inline friend Archive &operator<<(Archive &archive, LodInfo &lod_info) {
      archive << lod_info.index_count;
      archive << lod_info.vertex_count;
      archive << lod_info.has_colors;
      return archive;
    }
  };
  Vector<LodInfo> lod_info;
  math::Box bounds;

  float import_scale_factor = 1.f;

private:
  SharedPtr<SkinnedMeshResource> render_data;
};
} // namespace runtime
