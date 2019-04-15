add_library(
  preprocessor
  STATIC
  include/preprocessor/preprocessor.h
  src/preprocessor.cpp
)
target_include_directories(preprocessor PUBLIC include)
target_link_libraries(preprocessor PUBLIC graphicsinterface shader 
  core
  mcpp 
)
