#pragma once
#include <asset/factory.h>
#include <runtime/environment_map.h>

struct EnvironmentMapFactory : public asset::Factory {
  virtual const char *get_filename_extensions() override {
    return "iblarchive";
  }

  virtual asset::AssetRef create(const String &filename) override {
    auto &asset = *new runtime::EnvironmentMap;
    return asset::AssetRef(new runtime::EnvironmentMap);
  }

protected:
  virtual void load_asset_data(asset::Asset &asset) override;
};
