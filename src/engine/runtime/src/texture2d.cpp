#include <runtime/texture2d.h>
#include <renderer/resource.h>
#include <runtime/texture2d_resource.h>

using namespace runtime;

void Texture2D::serialize(Archive &archive) {
  Asset::serialize(archive);
  archive << texture_data;
}

renderer::Resource *Texture2D::create_resource() {
  return new Texture2DResource(this);
}

void Texture2D::init_resource() {
  assert(is_valid() || get_size_x() > 0);
  if (is_valid()) {
    if (!is_loaded_to_ram)
      asset::load_to_ram(*this, false, false); 
    resource = (Texture2DResource *)create_resource();
    return;
  }
  DEBUG_LOG(Rendering, Warning, "Runtime textures aren't supported");
}

Texture2DResource *Texture2D::get_resource() {
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

void Texture2D::update_resource() {
  DEBUG_LOG(System, Log, "update_resource was called");
  release_resource();
  init_resource();
}

void Texture2D::release_resource() {
  if (resource)
    delete resource;
  resource = nullptr;
}
implement_asset_type(Texture2D);
