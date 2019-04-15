#pragma once

#include <asset/factory.h>

using asset::Factory;
using asset::AssetRef;

struct ShaderFactory : public Factory {
  virtual const char *get_filename_extensions() override;
  virtual AssetRef create(const String &filename) override;
};
