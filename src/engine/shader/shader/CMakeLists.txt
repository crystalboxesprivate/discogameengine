add_library(
  shader
  STATIC
  include/shader/uniform_buffer.h
  include/shader/cache.h
  include/shader/shader.h
  include/shader/compiler.h
  src/uniform_buffer.cpp
  src/cache.cpp
  src/shader.cpp
)

target_include_directories(shader PUBLIC include)
target_link_libraries(shader PUBLIC core asset renderer cross config graphicsinterface)
