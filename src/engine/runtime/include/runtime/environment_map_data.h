#pragma once

#include <engine>
#include <graphicsinterface/pixel_format.h>

namespace runtime {
namespace environment_map {
struct ImageData {
  i32 size;
  graphicsinterface::PixelFormat pixel_format;
  Vector<u8> data;

  inline friend Archive &operator<<(Archive &archive, ImageData &image_data) {
    archive << image_data.size;
    archive << image_data.pixel_format;
    archive << image_data.data;
    return archive;
  }
};
} // namespace environment_map
struct EnvironmentMapData {
  i32 color_size = 0;
  i32 irradiance_size;
  i32 prefilter_size;
  i32 prefilter_num_mips;

  Vector<environment_map::ImageData> color_map;             // 6 sides
  Vector<environment_map::ImageData> irradiance_map;        // 6 sides
  Vector<Vector<environment_map::ImageData>> prefilter_map; // n mips * 6 sides

  inline friend Archive &operator<<(Archive &archive, EnvironmentMapData &env_data) {
    archive << env_data.color_size;
    archive << env_data.irradiance_size;
    archive << env_data.prefilter_size;
    archive << env_data.prefilter_num_mips;

    archive << env_data.color_map;
    archive << env_data.irradiance_map;
    archive << env_data.prefilter_map;
    return archive;
  }
};
} // namespace runtime
