add_library(
  d3d11i
  STATIC
  include/d3d11i/d3d11i.h
  include/d3d11i/d3dishader.h
  src/d3d11i.cpp
  src/d3dbuffer.cpp
  src/render_target.cpp
  src/shader_compilation.cpp
  src/rendering.cpp
  src/debug.cpp
  src/blending_state.cpp
  src/texturecube.cpp
)

target_include_directories(d3d11i PUBLIC include)
target_link_libraries(d3d11i PUBLIC core graphicsinterface shader)
target_link_libraries(d3d11i PRIVATE ntwindow)
