#pragma once
#include <asset/factory.h>
#include <runtime/texturecube.h>

struct TextureCubeFactory : public asset::Factory {
  virtual const char *get_filename_extensions() override {
    return "cubemapjson";
  }

  virtual asset::AssetRef create(const String &filename) override {
    auto &asset = *new runtime::TextureCube;
    return asset::AssetRef(new runtime::TextureCube);
  }

protected:
  virtual void load_asset_data(asset::Asset &asset) override;
};
