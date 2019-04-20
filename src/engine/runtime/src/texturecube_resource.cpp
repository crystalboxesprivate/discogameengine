#include <runtime/texturecube_resource.h>
#include <runtime/texturecube.h>
#include <graphicsinterface/graphicsinterface.h>

namespace gi = graphicsinterface;
using namespace runtime;
void TextureCubeResource::init() {
  needs_to_init = true;

  assert(owner->is_valid());
  assert(owner->is_loaded_to_ram || (!owner->is_loaded_to_ram && owner->is_loading));

  if (!owner->is_loaded_to_ram) {
    if (!owner->is_loading) {
      return;
    }
  }

  // this part of the code is called until the ram resource is ready.
  {

    u32 width = owner->get_size_x();
    u32 height = owner->get_size_y();

      //DEBUG_LOG(Rendering, Log, "Asset anomaly has 0 width but reads from the loaded asset");
    gi::PixelFormat pixel_format = owner->pixel_format;
    if (width == 0) {
      return;
    }

    texcube_color = gi::create_texture_cube(width, height, pixel_format, (void *)owner->texture_data.get_data());
    srv_color = gi::create_shader_resource_view(texcube_color);

    u32 byte_offset = width * height * get_byte_count_from_pixelformat(pixel_format) * 6; // 6 cubemap sides

    texcube_irradiance = gi::create_texture_cube(width, height, pixel_format, (void *)(owner->texture_data.get_data() + byte_offset));
    srv_irradiance = gi::create_shader_resource_view(texcube_irradiance);

    sampler_state = gi::create_sampler_state();
    // texture is loaded to gpu
    // freeing up the ram...
    asset::free(owner->hash);
    needs_to_init = false;
  }
}
