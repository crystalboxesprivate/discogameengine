add_library(
  renderer
  STATIC
  include/renderer/mesh_render_data.h
  include/renderer/resource.h
  include/renderer/rendercore.h
  include/renderer/renderer.h
  include/renderer/post_processing.h
  include/renderer/procedural_geo.h
  include/renderer/screen_space_quad.h
  src/renderer.cpp
  src/rendercore.cpp
  src/procedural_geo.cpp
  src/post_processing.cpp
  src/screen_space_quad.cpp
)
target_include_directories(renderer PUBLIC include)
target_link_libraries(renderer PUBLIC core asset graphicsinterface app)
target_link_libraries(renderer PRIVATE game shader config runtime input window task)
