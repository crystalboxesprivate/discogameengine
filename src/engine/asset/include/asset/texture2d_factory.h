#pragma once
#include <asset/factory.h>
#include <runtime/texture2d.h>

struct Texture2DFactory : public asset::Factory {
  virtual const char *get_filename_extensions() override {
    return "bmp;png;tga;jpg";
  }

  virtual asset::AssetRef create(const String &filename) override {
    auto &asset = *new runtime::Texture2D;
    return asset::AssetRef(new runtime::Texture2D);
  }

protected:
  virtual void load_asset_data(asset::Asset &asset) override;
};
