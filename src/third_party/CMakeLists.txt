# Configure assimp
option(
  BUILD_SHARED_LIBS 
  "Build package with shared libraries." 
  OFF
)

option(
  ASSIMP_BUILD_TESTS 
  "If the test suite for Assimp is built in addition to the library." 
  OFF
)

option(
  ASSIMP_BUILD_ASSIMP_TOOLS 
  "If the supplementary tools for Assimp are built in addition to the library." 
  OFF
)
add_subdirectory(assimp)

# configure bullet 3
add_library(
  bullet3
  INTERFACE
)
target_include_directories(bullet3 INTERFACE bullet3)

add_subdirectory(bullet3)
add_subdirectory(fmod)
add_subdirectory(freetype)
add_subdirectory(glad)
add_subdirectory(glfw)
add_subdirectory(glm)
add_subdirectory(ShaderConductor)
add_subdirectory(mcpp)
add_subdirectory(SPIRV-Cross)
add_subdirectory(rapidjson)
add_subdirectory(stb)
add_subdirectory(libtiff)
