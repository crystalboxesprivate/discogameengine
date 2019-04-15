add_library(
  runtime
  STATIC
  include/runtime/texture.h
  include/runtime/texture2d.h
  include/runtime/mesh.h
  include/runtime/static_mesh.h
  include/runtime/skinned_mesh.h
  include/runtime/static_mesh_resource.h
  src/texture2d.cpp
  src/static_mesh.cpp
)
target_include_directories(runtime PUBLIC include)
target_link_libraries(runtime PUBLIC core asset renderer)
target_link_libraries(runtime PRIVATE graphicsinterface)
