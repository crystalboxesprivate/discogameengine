#pragma once

#include <asset/asset.h>
#include <runtime/texture.h>
#include <graphicsinterface/graphicsinterface.h>
#include <engine>

namespace runtime {
struct TextureCubeResource;
struct TextureCube : public Texture, public asset::Asset {
  declare_asset_type(TextureCube);

  virtual void serialize(Archive &archive) override;
  virtual renderer::Resource *create_resource() override;
  virtual void init_resource() override;
  virtual void update_resource() override;
  virtual void release_resource() override;

  virtual u16 get_size_x() const override {
    return size_x;
  }
  virtual u16 get_size_y() const override {
    return size_y;
  }

  TextureCubeResource *get_resource();

  Blob texture_data;
  virtual void free() override {
    texture_data.free();
  }

  u16 size_x = 0;
  u16 size_y = 0;

  bool is_loaded_from_hdri = false;

  graphicsinterface::PixelFormat pixel_format = graphicsinterface::PixelFormat::Unknown;

private:
  TextureCubeResource *resource = nullptr;
};
} // namespace runtime
