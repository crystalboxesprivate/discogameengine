#pragma once

#include <asset/asset.h>
#include <runtime/mesh.h>

namespace renderer {
struct Resource;
}

namespace runtime {
struct StaticMeshResource;
struct StaticMesh : public asset::Asset {
  declare_asset_type(StaticMesh);
  StaticMesh() {
  }

  Blob bulk_data;
  virtual void free() override {
    bulk_data.free();
  }

  StaticMeshResource *get_render_resource();
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
  SharedPtr<StaticMeshResource> render_data;
};
} // namespace runtime
