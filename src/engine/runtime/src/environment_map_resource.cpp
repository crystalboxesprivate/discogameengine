#include <runtime/environment_map_resource.h>
#include <runtime/environment_map.h>
#include <graphicsinterface/graphicsinterface.h>

namespace gi = graphicsinterface;
using namespace runtime;
void EnvironmentMapResource::init() {
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

    // DEBUG_LOG(Rendering, Log, "Asset anomaly has 0 width but reads from the loaded asset");
    gi::PixelFormat pixel_format = owner->get_pixel_format();
    if (width == 0) {
      return;
    }

    if (owner->texture_data.color_map.size() != 6)
      return;
    if (owner->texture_data.irradiance_map.size() != 6)
      return;
    if (owner->texture_data.prefilter_map.size() != owner->texture_data.prefilter_num_mips)
      return;
    for (int x = 0; x < owner->texture_data.prefilter_num_mips; x++) {
      if (owner->texture_data.prefilter_map[x].size() != 6)
        return;
    }

    // texcube_color = gi::create_texture_cube(width, height, pixel_format, (void *)owner->texture_data.get_data());
    sampler_state = gi::create_sampler_state(gi::FILTER_MIN_MAG_MIP_LINEAR);
    auto &texture_data = owner->texture_data;
    color = gi::create_texture_cube(texture_data.color_size, texture_data.color_size, pixel_format, 0,
                                    "Environment (color)");
    irradiance = gi::create_texture_cube(texture_data.irradiance_size, texture_data.irradiance_size, pixel_format, 0,
                                         "Environment (irradiance)");
    prefilter = gi::create_texture_cube(texture_data.prefilter_size, texture_data.prefilter_size, pixel_format, -1,
                                        "Environment (prefilter)");

    for (int x = 0; x < 6; x++) {
      gi::update_subresource(color, x, 0, texture_data.color_map[x].size,
                             (void *)texture_data.color_map[x].data.data());
      gi::update_subresource(irradiance, x, 0, texture_data.irradiance_map[x].size,
                             (void *)texture_data.irradiance_map[x].data.data());
    }

    for (int mip = 0; mip < texture_data.prefilter_map.size(); mip++) {
      for (int x = 0; x < 6; x++) {
        auto data_ptr = texture_data.prefilter_map[mip][x].data.data();
        gi::update_subresource(prefilter, x, mip, texture_data.prefilter_map[mip][x].size, (void *)data_ptr);
      }
    }

    color_srv = gi::create_shader_resource_view(color);
    irradiance_srv = gi::create_shader_resource_view(irradiance);
    prefilter_srv = gi::create_shader_resource_view(prefilter);

    // texture is loaded to gpu
    // freeing up the ram...
    asset::free(owner->hash);
    needs_to_init = false;
  }
}
