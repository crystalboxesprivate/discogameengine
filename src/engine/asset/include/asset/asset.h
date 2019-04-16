#pragma once

#include <engine>
#include <utils/string.h>

#define declare_asset_type(type)                                                                                       \
  static constexpr usize get_type_id() {                                                                               \
    return utils::string::hash_code(#type);                                                                            \
  }                                                                                                                    \
  virtual void serialize_type_id(Archive &archive) override {                                                          \
    usize type_id = get_type_id();                                                                                     \
    archive << type_id;                                                                                                \
  }                                                                                                                    \
  static asset::AssetTypeBase *get_type_object();                                                                      \
  static asset::AssetType<type> type::component_##type;

#define implement_asset_type(type)                                                                                     \
  asset::AssetType<type> type::component_##type(type::get_type_id());                                                  \
  asset::AssetTypeBase *type::get_type_object() {                                                                      \
    return &component_##type;                                                                                          \
  }

namespace asset {
enum class Type : u8 { Texture2D, StaticMesh, Custom, Unknown };
struct AssetTypeBase;
struct Factory;
struct Asset {
  Asset()
      : type(Type::Custom)
      , is_loaded_to_ram(false)
      , is_loading(false)
      , keep_on_cpu(true)
      , load_to_gpu(false)
      , hash(0) {
  }

  virtual void serialize(Archive &archive) ;
  virtual void serialize_type_id(Archive &archive) = 0;

  inline bool is_valid() const {
    return hash != 0;
  }

  virtual void free() {
  }

  bool is_loaded_to_ram;
  bool is_loading;

  usize hash;

  Type type;

  bool load_to_gpu;
  bool keep_on_cpu;

  String source_filename;
  String alias;
};

typedef SharedPtr<Asset> AssetRef;

template <typename T>
struct AssetHandle {
  AssetHandle()
      : ref(nullptr)
      , hash(0) {
  }
  AssetHandle(AssetRef in_ref)
      : ref(in_ref)
      , hash(0) {
    if (!ref)
      return;
    hash = ref->hash;
  }
  usize hash;
  AssetRef ref;

  inline friend Archive &operator<<(Archive &archive, AssetHandle &reference) {
    archive << reference.hash;
    return archive;
  }

  AssetRef get_ref() {
    if (!hash)
      return nullptr;

    if (!ref)
      ref = asset::find(hash);

    return ref;
  }

  inline T *get() {
    return reinterpret_cast<T *>(get_ref().get());
  }
};

Archive &operator<<(Archive &archive, Type &asset_type);

void get_asset_cache_path(String &out_path, usize hash);

AssetRef add(const String &filename, const String &alias = "");
AssetRef find(usize hash);

template <typename T>
AssetHandle<T> add(const String &filename, const String &alias = "") {
  auto ref = add(filename, alias);
  return AssetHandle<T>(ref);
}

// Type get_type_from_filename(const String &filename);
Asset &get_default_asset(Type asset_type);

template <typename T>
T &get_default(Type asset_type) {
  T &asset_of_type = *reinterpret_cast<T *>(&get_default_asset(asset_type));
  asset_of_type.is_valid();
  return asset_of_type;
}

void load_to_ram(Asset &asset, bool force_rebuild = false);
void load_to_ram(AssetRef asset, bool force_rebuild = false);

void resave_to_disk(Asset &asset);
void resave_to_disk(AssetRef asset);

void free(usize asset_hash);

struct AssetTypeBase {
  static AssetTypeBase *asset_types[256];
  static usize current_size;
  virtual AssetRef make_new() = 0;
  usize asset_type_id;
};

template <typename T>
struct AssetType : public AssetTypeBase {
  AssetType(usize in_type_id) {
    asset_types[current_size++] = this;
    asset_type_id = in_type_id;
  }
  virtual AssetRef make_new() override {
    return AssetRef(new T);
  }
};

AssetRef make_new(usize type_id);

template <typename T>
inline AssetRef make_new() {
  return make_new(T::get_type_id());
}
} // namespace asset
