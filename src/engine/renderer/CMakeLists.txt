add_library(
  renderer
  STATIC
  include/renderer/mesh_render_data.h
  include/renderer/resource.h
  src/renderer.cpp
)
target_include_directories(renderer PUBLIC include)
target_link_libraries(renderer PUBLIC core asset graphicsinterface app)
target_link_libraries(renderer PRIVATE game shader config runtime input window task)
