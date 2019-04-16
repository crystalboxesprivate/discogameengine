configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.in ${CMAKE_BINARY_DIR}/include/config/config.h)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/shader_target.h.in ${CMAKE_BINARY_DIR}/include/config/shader_target.h)
add_library(config INTERFACE)
target_include_directories(config INTERFACE ${CMAKE_BINARY_DIR}/include)
