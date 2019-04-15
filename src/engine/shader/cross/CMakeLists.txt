add_library(
  cross
  STATIC
  include/cross/cross.h
  src/cross.cpp
)
target_include_directories(cross PUBLIC include)
target_link_libraries(cross PUBLIC graphicsinterface shader preprocessor
  core
  ShaderConductor 
  spirv-cross-core
  spirv-cross-glsl
  spirv-cross-cpp
  spirv-cross-reflect
  spirv-cross-util
)
