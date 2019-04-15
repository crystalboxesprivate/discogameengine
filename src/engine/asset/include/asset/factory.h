#pragma once

#include <engine>
#include <asset/asset.h>
#include <utils/path.h>

#define implement_asset_factory(type) static asset::factory::TypedAssetFactory<type> g_asset_factory_##type;

namespace asset {

struct Factory {
  virtual const char *get_filename_extensions() = 0;
  virtual void load(Asset& asset, bool force_rebuild);
  virtual AssetRef create(const String &filename) = 0;

protected:
  virtual void load_asset_data(Asset& asset) {
  }
};
namespace factory {
struct AssetFactory {
  virtual Factory *get() = 0;
  static AssetFactory *factories[128];
  static usize current_size;
};
template <typename T>
struct TypedAssetFactory : public AssetFactory {
  TypedAssetFactory() {
    constructor_id = current_size;
    factories[current_size++] = this;
  }
  virtual Factory *get() override {
    return &factory;
  }
  T factory;
  usize constructor_id;
};
} // namespace factory

Factory *find_factory(const String &filename);
void register_factory(factory::AssetFactory *factory);
} // namespace asset
