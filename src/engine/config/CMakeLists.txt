configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.in ${CMAKE_BINARY_DIR}/include/config/config.h)
add_library(config INTERFACE)
target_include_directories(config INTERFACE ${CMAKE_BINARY_DIR}/include)
