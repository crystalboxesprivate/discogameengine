add_executable(
  engine WIN32
  WinMain.cpp
)

add_executable(compiler_test main.cpp)

# set(ENGINE_GRAPHICS_INTERFACE opengli)
# set(ENGINE_SHADER_TARGET_LANG Glsl)

set(ENGINE_GRAPHICS_INTERFACE d3d11i)
set(ENGINE_SHADER_TARGET_LANG Hlsl)


set(ENGINE_CACHE_ENABLED 0)
set(ENGINE_DEBUG_GRAPHICS 1)
set(ENGINE_DEBUG_D3D11LAYER 1)
set(ENGINE_SHADER_FORCE_RECOMPILE 1)
set(ENGINE_FORCE_SYNC_ASSET_LOADING 0)

add_subdirectory(task)
add_subdirectory(core)
add_subdirectory(config)
add_subdirectory(utils)
add_subdirectory(interfaces)
add_subdirectory(app)
add_subdirectory(component)
add_subdirectory(game)
add_subdirectory(asset)
add_subdirectory(legacy)
add_subdirectory(renderer)
add_subdirectory(runtime)
add_subdirectory(platform)
add_subdirectory(shader)
add_subdirectory(behavior)

target_link_libraries(engine PUBLIC config core app legacy utils)
target_link_libraries(engine PRIVATE task behavior ntwindow component game renderer shader)

set_target_properties(engine PROPERTIES OUTPUT_NAME disco-engine
  RUNTIME_OUTPUT_DIRECTORY ${ENGINE_BINARY_DIR})
  
target_link_libraries(compiler_test PUBLIC config core app legacy utils shader asset)
set_target_properties(compiler_test PROPERTIES OUTPUT_NAME compiler_test
  RUNTIME_OUTPUT_DIRECTORY ${ENGINE_BINARY_DIR})
target_link_libraries(compiler_test PRIVATE task behavior ntwindow component game preprocessor)


add_custom_command(
  TARGET engine
  POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_if_different "${ENGINE_PROJECT_DIR}/src/third_party/assimp/lib/assimp-vc140-mt.dll" "$<TARGET_FILE_DIR:engine>"
  COMMAND ${CMAKE_COMMAND} -E copy_if_different "${ENGINE_PROJECT_DIR}/src/third_party/fmod/lib/fmod.dll" "$<TARGET_FILE_DIR:engine>"
  COMMAND ${CMAKE_COMMAND} -E copy_if_different "${ENGINE_PROJECT_DIR}/src/third_party/freetype/lib/freetype271MT.dll" "$<TARGET_FILE_DIR:engine>"
  COMMAND ${CMAKE_COMMAND} -E copy_if_different "${ENGINE_PROJECT_DIR}/src/third_party/ShaderConductor/lib/dxcompiler.dll" "$<TARGET_FILE_DIR:engine>"
  COMMAND ${CMAKE_COMMAND} -E copy_if_different "${ENGINE_PROJECT_DIR}/src/third_party/ShaderConductor/lib/ShaderConductor.dll" "$<TARGET_FILE_DIR:engine>"
)
