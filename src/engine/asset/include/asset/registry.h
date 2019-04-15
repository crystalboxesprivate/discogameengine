#pragma once

#include <engine>
#include <asset/asset.h>

namespace asset {
  struct Factory;
  struct Registry {
    HashMap <usize, AssetRef> assets;
    HashMap <Type, AssetRef> default_assets;
  };
}
