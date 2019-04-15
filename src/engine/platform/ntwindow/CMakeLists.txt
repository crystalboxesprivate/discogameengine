add_library(
  ntwindow
  STATIC
  include/ntwindow/ntwindow.h
  include/ntwindow/game_timer.h
  src/ntwindow.cpp
  src/nt_init.cpp
  src/game_timer.cpp
)

target_include_directories(ntwindow PUBLIC include)
target_link_libraries(ntwindow PUBLIC core window graphicsinterface input)
target_link_libraries(ntwindow PRIVATE ${ENGINE_GRAPHICS_INTERFACE})
