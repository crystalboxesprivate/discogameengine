#pragma once

#include <asset/asset.h>
#include <runtime/texture.h>
#include <graphicsinterface/graphicsinterface.h>
#include <engine>
#include <runtime/environment_map_data.h>

namespace runtime {
struct EnvironmentMapResource;
struct EnvironmentMap : public Texture, public asset::Asset {
  declare_asset_type(EnvironmentMap);

  virtual void serialize(Archive &archive) override;
  virtual renderer::Resource *create_resource() override;
  virtual void init_resource() override;
  virtual void update_resource() override;
  virtual void release_resource() override;

  virtual u16 get_size_x() const override {
    return texture_data.color_size;
  }
  virtual u16 get_size_y() const override {
    return texture_data.color_size;
  }

  virtual graphicsinterface::PixelFormat get_pixel_format() const override {
    return texture_data.color_map.size() ? texture_data.color_map[0].pixel_format
                                         : graphicsinterface::PixelFormat::Unknown;
  }

  EnvironmentMapResource *get_resource();

  EnvironmentMapData texture_data;
  virtual void free() override {
    texture_data.color_map.clear();
    texture_data.color_map.shrink_to_fit();

    texture_data.irradiance_map.clear();
    texture_data.irradiance_map.shrink_to_fit();

    texture_data.prefilter_map.clear();
    texture_data.irradiance_map.shrink_to_fit();
  }

private:
  EnvironmentMapResource *resource = nullptr;
};
} // namespace runtime
