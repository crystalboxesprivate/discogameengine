#include <runtime/texture2d.h>
#include <runtime/texture2d_resource.h>
#include <graphicsinterface/graphicsinterface.h>

namespace gi = graphicsinterface;
using namespace runtime;
void Texture2DResource::init() {
  needs_to_init = true;

  assert(owner->is_valid());
  assert(owner->is_loaded_to_ram || (!owner->is_loaded_to_ram && owner->is_loading));

  if (!owner->is_loaded_to_ram) {
    if (!owner->is_loading) {
      return;
    }
  }

  // this part of the code is called until the ram resource is ready...
  {

    auto width = owner->get_size_x();
    auto height = owner->get_size_y();
    auto pixel_format = owner->pixel_format;
    if (width == 0) {
      //DEBUG_LOG(Rendering, Log, "Asset anomaly has 0 width but reads from the loaded asset");
      return;
    }
    //assert(pixel_format != gi::PixelFormat::Unknown);
    //assert(width);
    //assert(height);
    // it's loaded _. create render resource load texture blob into vram
    texture2d = gi::create_texture2d(width, height, pixel_format, (void *)owner->texture_data.get_data());
    sampler_state = gi::create_sampler_state();
    srv = gi::create_shader_resource_view(texture2d);
    // texture is loaded to gpu
    // freeing up the ram...
    asset::free(owner->hash);
    needs_to_init = false;
  }
}
