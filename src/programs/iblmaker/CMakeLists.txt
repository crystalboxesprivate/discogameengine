set(sources
  include/cubemap_builder.h
  include/rendering_utils.h
  include/shaders.h
  src/main.cpp
  src/cubemap_builder.cpp
  src/rendering_utils.cpp
)

include_directories(include)
add_executable(iblmaker ${sources})
target_include_directories(iblmaker PRIVATE include)
target_link_libraries(iblmaker glfw glad stb glm runtime)

set_target_properties(iblmaker PROPERTIES OUTPUT_NAME iblmaker
  RUNTIME_OUTPUT_DIRECTORY ${ENGINE_BINARY_DIR})