#include <asset/texture2d_factory.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>
#include <utils/path.h>
#include <tiff/tiffio.h>

using namespace runtime;

void tiff_error_handler(const char *module, const char *fmt, va_list ap) {
  DEBUG_LOG(Assets, Log, fmt, ap);
}

void Texture2DFactory::load_asset_data(asset::Asset &asset) {
  Texture2D &texture2d = *static_cast<Texture2D *>(&asset);

  String extension = utils::path::get_extension(texture2d.source_filename);
  i32 width = 0, height = 0, num_bytes = 0;

  if (extension == "tif") {
    // const aiScene *scene = aiImportFile(texture2d.source_filename.c_str(), 0);
    // DEBUG_LOG(Assets, Log, "%d", scene->mNumMeshes);
    TIFFSetWarningHandler(tiff_error_handler);
    TIFF *tif = TIFFOpen(texture2d.source_filename.c_str(), "r");
    assert(tif);
    {
      u32 w, h;
      TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &w);
      TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &h);
      width = w;
      height = h;
    }

    usize npixels = width * height;
    u32 *raster = (u32 *)_TIFFmalloc(npixels * sizeof(u32));
    if (raster != NULL) {
      if (TIFFReadRGBAImage(tif, width, height, raster, 0)) {
        //... process raster data...
        texture2d.texture_data.allocate(npixels * 4, raster);
      }
      _TIFFfree(raster);
    }

    TIFFClose(tif);
  } else {
    i32 nrChannels = 0;
    u8 *data = stbi_load(texture2d.source_filename.c_str(), &width, &height, &nrChannels, 4);
    assert(width);
    assert(height);
    num_bytes = width * height * 4;
    texture2d.texture_data.allocate(num_bytes, data);
    stbi_image_free(data);
  }

  texture2d.size_x = width;
  texture2d.size_y = height;
  texture2d.pixel_format = graphicsinterface::PixelFormat::R8G8B8A8F;
}
