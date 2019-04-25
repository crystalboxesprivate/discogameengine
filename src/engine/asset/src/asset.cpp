#include <cassert>
#include <functional>

#include <asset/registry.h>
#include <asset/asset.h>
#include <asset/factory.h>
#include <utils/path.h>
#include <utils/string.h>

#include <app/app.h>
#include <utils/map.h>
#include <utils/path.h>
#include <utils/helpers.h>
#include <config/config.h>
#include <core/queue.h>
#include <thread>
#include <mutex>

namespace asset {
const Asset *get_default_asset(Type asset_type) {
  // DEBUG_LOG(Assets, Error, "not implemented initialization of default assets");
  return nullptr;
  Asset *asset = app::get().get_asset_registry().default_assets[asset_type].get();
  ;
  return asset;
}

inline usize make_hash(const String &filename) {
  return utils::string::hash_code(filename);
}

const AssetRef find(usize hash) {
  auto &assets_map = app::get().get_asset_registry().assets;
  AssetRef *asset = helpers::find<usize, AssetRef>(assets_map, hash);
  if (asset) {
    return *asset;
  }
  return nullptr;
}

void get_asset_cache_path(String &out_path, usize hash) {
  String filename = std::to_string(hash);
  out_path = utils::path::join(config::CACHE_DIR, filename);
}

const AssetRef add(const String &filename, const String &alias) {
  if (!utils::path::exists(filename)) {
    DEBUG_LOG(Assets, Error, "Invalid asset path %s", filename.c_str());
    return false;
  }

  // It's already in the registry
  usize hash = make_hash(alias.size() ? filename + alias : filename);
  auto &assets_map = app::get().get_asset_registry().assets;
  {
    if (assets_map.size()) {
      AssetRef *asset = helpers::find<usize, AssetRef>(assets_map, hash);
      if (asset) {
        return *asset;
      }
    }
  }

  Factory *asset_factory;
  asset_factory = find_factory(filename);
  if (!asset_factory) {
    DEBUG_LOG(Assets, Error, "Couldn't find asset factory for: %s", filename.c_str());
    return false;
  }

  // String cached_path;
  AssetRef new_asset = asset_factory->create(filename); // : asset_factory->create(filename);
  assert(new_asset);

  // Add a newly created asset to the global registry.
  assets_map[hash] = new_asset;
  new_asset->hash = hash;
  new_asset->source_filename = filename;
  new_asset->alias = alias;

  return new_asset;
}

void Asset::serialize(Archive &archive) {
  serialize_type_id(archive);
  archive << hash;
  archive << source_filename;
  archive << alias;
}

Archive &operator<<(Archive &archive, Type &asset_type) {
  archive.serialize(&asset_type, sizeof(asset::Type));
  return archive;
}

void resave_to_disk(const Asset &asset) {
#if CACHE_TO_DISK

  auto &assets_map = app::get().get_asset_registry().assets;
  auto &result = *assets_map.find(asset.hash)->second.get();

  Archive archive;
  result.serialize(archive);
  volatile usize capacity = archive.get_current_offset();

  archive = Archive(capacity);
  result.serialize(archive);

  String filename;
  get_asset_cache_path(filename, result.hash);

  FILE *write_ptr;
  write_ptr = fopen(filename.c_str(), "wb");
  assert(write_ptr);
  fwrite(archive.data(), 1, archive.get_size(), write_ptr);
  fclose(write_ptr);
#endif
}

void resave_to_disk(const AssetRef asset) {
  assert(asset);
  resave_to_disk(*asset.get());
}
std::mutex m_;
struct AssetThread {
  struct AssetTask {
    Asset *asset;
    bool force_rebuild;
  };

  void add(AssetTask asset) {
    asset.asset->is_loading = true;
    asset.asset->is_loaded_to_ram = false;

    assets_to_load.enqueue(asset);
    if (!is_running) {
      std::thread t1(AssetThread::process_assets, this);
      t1.detach();
    }
  }

  static void process_assets(AssetThread *instance) {
    instance->is_running = true;

    while (!instance->assets_to_load.is_empty()) {
      auto asset_task = instance->assets_to_load.dequeue();
      AssetThread::load(*asset_task.asset, asset_task.force_rebuild);
    }

    instance->is_running = false;
  }

  static void load(Asset &asset, bool force_rebuild) {
    {

      Factory *asset_factory = find_factory(asset.source_filename);
      assert(asset_factory);
      m_.lock();
      (*asset_factory).load(asset, force_rebuild);
      m_.unlock();
      asset.is_loading = false;
      asset.is_loaded_to_ram = true;
    }
  }
  Queue<AssetTask> assets_to_load;
  bool is_running = false;
};

static const i32 NUM_ASSET_THREADS = 6;
AssetThread asset_thread[NUM_ASSET_THREADS];
i32 current_asset_thread = 0;

AssetThread& get_asset_thread() {
  current_asset_thread = (current_asset_thread + 1) % NUM_ASSET_THREADS;
  DEBUG_LOG(Assets, Log, "get asset thread: %d", current_asset_thread);
  return asset_thread[current_asset_thread];
}

void load_to_ram(const AssetRef asset, bool force_rebuild, bool load_immediately) {
  load_to_ram(*asset.get(), force_rebuild, load_immediately);
}

void load_to_ram(const Asset &asset, bool force_rebuild, bool load_immediately) {
  if (asset.is_loaded_to_ram || asset.is_loading)
    return;
  #if ENGINE_FORCE_SYNC_ASSET_LOADING
  load_immediately = true;
  #endif
  
  auto &assets_map = app::get().get_asset_registry().assets;
  auto result = assets_map.find(asset.hash)->second.get();

  if (load_immediately) {
    AssetThread::load(*result, force_rebuild);
  } else {
    get_asset_thread().add({result, force_rebuild});
  }
}

void free(usize asset_hash) {
  auto &assets_map = app::get().get_asset_registry().assets;
  auto &result = assets_map.find(asset_hash);

  assert(result != assets_map.end());

  //result->second->free();
}

AssetTypeBase *AssetTypeBase::asset_types[256];
usize AssetTypeBase::current_size;

AssetRef make_new(usize type_id) {
  for (int x = 0; x < AssetTypeBase::current_size; x++) {
    if (AssetTypeBase::asset_types[x]->asset_type_id == type_id) {
      return AssetTypeBase::asset_types[x]->make_new();
    }
  }
  assert(false);
  return nullptr;
}
} // namespace asset
