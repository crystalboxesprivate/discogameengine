#include <runtime/texturecube.h>
#include <renderer/resource.h>
#include <runtime/texturecube_resource.h>

using namespace runtime;

void TextureCube::serialize(Archive &archive) {
  Asset::serialize(archive);
  archive << texture_data;
}

renderer::Resource *TextureCube::create_resource() {
  return new TextureCubeResource(this);
}

void TextureCube::init_resource() {
  assert(is_valid() || get_size_x() > 0);
  if (is_valid()) {
    if (!is_loaded_to_ram)
      asset::load_to_ram(*this, false, false); 
    resource = (TextureCubeResource *)create_resource();
    return;
  }
  DEBUG_LOG(Rendering, Warning, "Runtime textures aren't supported");
}

TextureCubeResource *TextureCube::get_resource() {
  if (!resource) {
    update_resource();
    return nullptr;
  }
  if (resource->needs_to_init) {
    resource->init();
    return nullptr;
  }

  return resource;
}

void TextureCube::update_resource() {
  DEBUG_LOG(System, Log, "update_resource was called");
  release_resource();
  init_resource();
}

void TextureCube::release_resource() {
  if (resource)
    delete resource;
  resource = nullptr;
}
implement_asset_type(TextureCube);
