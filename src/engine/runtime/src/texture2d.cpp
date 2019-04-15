#include <runtime/texture2d.h>
#include <renderer/resource.h>

using namespace runtime;


struct Texture2DResource : public renderer::Resource {
  Texture2DResource(Texture2D &in_texture)
      : owner(in_texture)
      , native_resource(nullptr)
      , needs_to_init(false) {
  }

  virtual void *get_graphics_resource() override {
    if (needs_to_init)
      init();

    return native_resource;
  }

  virtual void init() override {
    needs_to_init = true;

    assert(owner.is_valid());
    assert(owner.is_loaded_to_ram || (!owner.is_loaded_to_ram && owner.is_loading));

    if (!owner.is_loading) {
      return;
    }

    // this part of the code is called until the ram resource is ready...
    {
      // it's loaded _. create render resource load texture blob into vram
      assert(false && "hardware interface logic");
      // texture is loaded to gpu
      // freeing up the ram...
      asset::free(owner.hash);
      needs_to_init = false;
    }
  }

  virtual void release() override {
    assert(false && "hardware interface logic");
  }

  const Texture2D &owner;
  void *native_resource;
  bool needs_to_init;
};

void Texture2D::serialize(Archive &archive) {
  Asset::serialize(archive);
}

renderer::Resource *Texture2D::create_resource() {
  return new Texture2DResource(*this);
}

void Texture2D::init_resource() {
  assert(is_valid() || get_size_x() > 0);
  if (is_valid()) {
    if (!is_loaded_to_ram)
      asset::load_to_ram(*this); // triggers "is loading..."
    create_resource();
    return;
  }
  DEBUG_LOG(Rendering, Warning, "Runtime textures aren't supported");
}

void Texture2D::update_resource() {
  DEBUG_LOG(System, Log, "update_resource was called");
  release_resource();
}

void Texture2D::release_resource() {
  if (resource)
    delete resource;
  resource = nullptr;
}
implement_asset_type(Texture2D);
