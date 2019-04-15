cmake_minimum_required(VERSION 3.12)
project(disco_engine)

macro(add_pre_built_static_library name lib_path)
  add_library(
    ${name} STATIC
    ${lib_path}
  )
  set_target_properties(${name} PROPERTIES LINKER_LANGUAGE C++)
  target_include_directories(${name} PUBLIC include)
  add_custom_command(TARGET ${name} POST_BUILD 
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/${lib_path} "$<TARGET_FILE_DIR:${name}>"
  )
endmacro()

if(WIN32)
  if(MSVC)
      add_definitions(-D_CRT_SECURE_NO_WARNINGS)
  endif()
endif()

set (ENGINE_BINARY_DIR "${CMAKE_CURRENT_SOURCE_DIR}/binary")
set (ENGINE_PROJECT_DIR "${PROJECT_SOURCE_DIR}")

add_subdirectory(src)
