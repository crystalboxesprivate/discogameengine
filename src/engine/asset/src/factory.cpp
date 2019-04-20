#include <asset/factory.h>
#include <asset/registry.h>
#include <app/app.h>
#include <utils/helpers.h>
#include <utils/path.h>

#include <asset/texture2d_factory.h>
#include <asset/static_mesh_factory.h>
#include <asset/shader_factory.h>
#include <asset/texturecube_factory.h>

#include <config/config.h>

namespace asset {

HashMap<u32, Factory *> factories;

void Factory::load(Asset &asset, bool force_rebuild) {
  assert(asset.is_valid());
  String cache_path;
  asset::get_asset_cache_path(cache_path, asset.hash);
#if CACHE_TO_DISK
  bool is_cached = utils::path::exists(cache_path);
  if (is_cached && !force_rebuild) {
    Archive ar(cache_path);
    asset.serialize(ar);
    return;
  }
#endif
  load_asset_data(asset);
  resave_to_disk(asset);
}

// https://stackoverflow.com/a/7666577
u32 str_hash(const char *str) {
  u32 hash = 5381;
  int c;

  while (c = *str++)
    hash = ((hash << 5) + hash) + c;

  return hash;
}

u32 str_hash(const String &str) {
  u32 hash = 5381;

  for (int x = 0; x < str.size(); x++) {
    hash = ((hash << 5) + hash) + str[x];
  }

  return hash;
}
namespace factory {
AssetFactory *AssetFactory::factories[128];
usize AssetFactory::current_size = 0;
} // namespace factory
  // static AssetFactory *factories[128];
  // static usize current_size;
void register_factory(factory::AssetFactory *factory) {
  asset::factory::AssetFactory::factories[asset::factory::AssetFactory::current_size++] = factory;
  // auto &registry = app::get().get_asset_registry();
}

void init_factories() {
  for (int z = 0; z < asset::factory::AssetFactory::current_size; z++) {
    Factory &factory = *asset::factory::AssetFactory::factories[z]->get();

    const char *extensions = factory.get_filename_extensions();
    char buf[24];
    usize len = strlen(extensions);
    for (int x = 0, y = 0; x < len + 1; x++, y++) {
      if (extensions[x] == ';' || x == len) {
        buf[y] = '\0';
        factories[str_hash(buf)] = &factory;
        y = -1;
        continue;
      }
      buf[y] = extensions[x];
    }
  }
}

Factory *find_factory(const String &filename) {
  if (factories.size() != asset::factory::AssetFactory::current_size)
    init_factories();

  auto ext = utils::path::get_extension(filename);
  auto factory = helpers::find<u32, Factory *>(factories, str_hash(ext));
  if (!factory)
    return nullptr;
  return *factory;
}
} // namespace asset
implement_asset_factory(Texture2DFactory);
implement_asset_factory(TextureCubeFactory);
implement_asset_factory(StaticMeshFactory);
implement_asset_factory(ShaderFactory);
