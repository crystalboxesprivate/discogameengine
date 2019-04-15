add_library(
  opengli
  STATIC
  include/opengli/opengli.h
  src/opengli_win.cpp
  src/buffer.cpp
)

target_include_directories(opengli PUBLIC include)
target_link_libraries(opengli PUBLIC core graphicsinterface)
target_link_libraries(opengli PRIVATE glad)
