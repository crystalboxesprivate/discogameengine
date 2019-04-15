#pragma once

#include <asset/asset.h>
#include <runtime/texture.h>
#include <engine>

namespace runtime {
struct Texture2D : public Texture, public asset::Asset {
  declare_asset_type(Texture2D);
  
  virtual void serialize(Archive &archive) override;
  virtual renderer::Resource *create_resource() override;
  virtual void init_resource() override;
  virtual void update_resource() override;
  virtual void release_resource() override;

  virtual usize get_size_x() override {
    assert(false);
    return 0;
  }
  virtual usize get_size_y() override {
    assert(false);
    return 0;
  }

};
} // namespace renderer
