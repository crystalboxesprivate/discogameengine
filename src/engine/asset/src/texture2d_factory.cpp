#include <asset/texture2d_factory.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

using namespace runtime;
void Texture2DFactory::load_asset_data(asset::Asset &asset) {
  Texture2D &texture2d = *static_cast<Texture2D *>(&asset);

  int width, height, nrChannels;
  unsigned char *data = stbi_load(texture2d.source_filename.c_str(), &width, &height, &nrChannels, 4);
  i32 num_bytes = width * height * 4;
  texture2d.texture_data.allocate(num_bytes, data);
  stbi_image_free(data);

  texture2d.size_x = width;
  texture2d.size_y = height;

  texture2d.pixel_format = graphicsinterface::PixelFormat::R8G8B8A8F;

  //assert(nrChannels == 4);
}
