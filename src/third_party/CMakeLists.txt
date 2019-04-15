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
# add_subdirectory(glfw-3.2.1)
add_subdirectory(glm)
add_subdirectory(ShaderConductor)
add_subdirectory(mcpp)
add_subdirectory(SPIRV-Cross)
add_subdirectory(rapidjson)
