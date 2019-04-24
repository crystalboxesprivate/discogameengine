add_library(
  asset
  STATIC
  include/asset/asset.h
  include/asset/factory.h
  include/asset/texture2d_factory.h
  include/asset/static_mesh_factory.h
  include/asset/shader_factory.h
  include/asset/environment_map_factory.h
  include/asset/skinned_mesh_factory.h
  src/asset.cpp
  src/factory.cpp
  src/shader_factory.cpp
  src/static_mesh_factory.cpp
  src/texture2d_factory.cpp
  src/environment_map_factory.cpp
  src/skinned_mesh_factory.cpp
)
target_include_directories(asset PUBLIC include)
target_link_libraries(asset PRIVATE app core runtime shader cross config assimp stb rapidjson tiff)
