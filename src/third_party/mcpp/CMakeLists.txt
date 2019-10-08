cmake_minimum_required(VERSION 3.14)
project(mcpp)
add_library(mcpp STATIC
  include/mcpp/mcpp.h
  src/directive.c
  src/eval.c
  src/expand.c
  src/main.c
  src/mbchar.c
  src/preproc.c
  src/support.c
  src/system.c
  src/configed.H
  src/internal.H
  src/mcpp_lib.h
  src/mcpp_out.h
  src/noconfig.H
  src/system.H
  src/config.h
)

if ( CMAKE_COMPILER_IS_GNUCC )
    target_compile_options(mcpp PRIVATE "-Wall -Wextra")
endif()

if ( MSVC )
    target_compile_options(mcpp PRIVATE "/W4")
endif()

set (COMPILE_DEFS 
  HAVE_CONFIG_H
  MCPP_LIB=1
  CPU="${CMAKE_HOST_SYSTEM_PROCESSOR}"
  VERSION="2.7.2"
)

target_include_directories(mcpp PUBLIC include)
target_include_directories(mcpp PRIVATE src)

if(UNIX AND NOT APPLE)
  set(LINUX TRUE)
endif()

if(WIN32)
  set (COMPILE_DEFS 
    ${COMPILE_DEFS}
    _WIN32_WINNT=0x600
    WIN32_LEAN_AND_MEAN
  )
elseif(APPLE)
  set_target_properties(mcpp PROPERTIES COMPILE_FLAGS 
    "-fno-common"
    "-stdlib=libstdc++"
    "-w"
  )
elseif(LINUX)
  set_target_properties(mcpp PROPERTIES COMPILE_FLAGS 
    "-fPIC"
    "-w"
  )
endif()

target_compile_definitions(mcpp PRIVATE ${COMPILE_DEFS})
