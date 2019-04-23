#include <asset/environment_map_factory.h>
#include <stb/stb_image.h>
#include <rapidjson/document.h>
#include <utils/fs.h>

using namespace runtime;
void EnvironmentMapFactory::load_asset_data(asset::Asset &asset) {
  TextureCube &texcube = *static_cast<TextureCube *>(&asset);
  String json_path = texcube.source_filename;
  String cubemap_dir = utils::path::parent(json_path);

  bool is_hdr = false;

  constexpr usize CUBEMAPS_NUM = 6;
  String color_paths[CUBEMAPS_NUM];
  String irradiance_paths[CUBEMAPS_NUM];
  {
    using namespace rapidjson;
    Document document;
    String json_content = utils::fs::load_file_to_string(json_path);
    assert(!document.Parse<kParseStopWhenDoneFlag>(json_content.c_str()).HasParseError());
    assert(document.HasMember("hdr"));
    assert(document.HasMember("color"));
    assert(document.HasMember("irradiance"));
    is_hdr = document["hdr"].GetBool();
    {
      using namespace utils::path;
      auto &color = document["color"];
      color_paths[0] = join(cubemap_dir, color["x+"].GetString());
      color_paths[1] = join(cubemap_dir, color["x-"].GetString());
      color_paths[2] = join(cubemap_dir, color["y+"].GetString());
      color_paths[3] = join(cubemap_dir, color["y-"].GetString());
      color_paths[4] = join(cubemap_dir, color["z+"].GetString());
      color_paths[5] = join(cubemap_dir, color["z-"].GetString());

      auto &irradiance = document["irradiance"];
      irradiance_paths[0] = join(cubemap_dir, irradiance["x+"].GetString());
      irradiance_paths[1] = join(cubemap_dir, irradiance["x-"].GetString());
      irradiance_paths[2] = join(cubemap_dir, irradiance["y+"].GetString());
      irradiance_paths[3] = join(cubemap_dir, irradiance["y-"].GetString());
      irradiance_paths[4] = join(cubemap_dir, irradiance["z+"].GetString());
      irradiance_paths[5] = join(cubemap_dir, irradiance["z-"].GetString());
    }
  }

  i32 cubemap_width, cubemap_height, cubemap_nrChannels;
  i32 bytes_per_pixel = is_hdr ? 16 : 4;
  i32 bytes_per_image = 0;
  constexpr i32 IMAGE_COUNT = 12; // 6 for color, 6 for irradiance map
  //assert(!is_hdr);

  // Load the first image separately to determine its dimensions
  {
    u8 *data = nullptr;
    const char *path = color_paths[0].c_str();
    if (is_hdr) {
      data = (u8 *)stbi_loadf(path, &cubemap_width, &cubemap_height, &cubemap_nrChannels, 4);
    } else {
      data = stbi_load(path, &cubemap_width, &cubemap_height, &cubemap_nrChannels, 4);
    }
    assert(data);
    bytes_per_image = cubemap_width * cubemap_height * bytes_per_pixel;
    texcube.texture_data.allocate(bytes_per_image * IMAGE_COUNT, nullptr);
    memcpy(texcube.texture_data.get_data(), data, bytes_per_image);
    stbi_image_free(data);
  }

  i32 width, height, channels;
  for (i32 x = 1; x < CUBEMAPS_NUM; x++) {
    i32 byte_offset = bytes_per_image * x;
    const char *path = color_paths[x].c_str();
    u8 *data = nullptr;
    if (is_hdr) {
      data = (u8 *)stbi_loadf(path, &width, &height, &channels, 4);
    } else {
      data = stbi_load(path, &width, &height, &channels, 4);
    }
    assert(data);
    assert(width == cubemap_width);
    assert(height == cubemap_height);
    memcpy(texcube.texture_data.get_data() + byte_offset, data, bytes_per_image);
    stbi_image_free(data);
  }

  // load irradiance maps
  for (i32 x = 0; x < CUBEMAPS_NUM; x++) {
    i32 byte_offset = bytes_per_image * (x + CUBEMAPS_NUM);
    const char *path = irradiance_paths[x].c_str();
    u8 *data = nullptr;
    if (is_hdr) {
      data = (u8 *)stbi_loadf(path, &width, &height, &channels, 4);
    } else {
      data = stbi_load(path, &width, &height, &channels, 4);
    }
    assert(data);
    assert(width == cubemap_width);
    assert(height == cubemap_height);
    memcpy(texcube.texture_data.get_data() + byte_offset, data, bytes_per_image);
    stbi_image_free(data);
  }

  {
    using namespace graphicsinterface;
    texcube.pixel_format = is_hdr ? PixelFormat::FloatRGBA : PixelFormat::R8G8B8A8F;
  }

  texcube.size_x = cubemap_width;
  texcube.size_y = cubemap_height;
}
