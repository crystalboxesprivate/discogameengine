add_library(
  app
  STATIC
  include/app/app.h
  src/app.cpp
)
target_include_directories(app PUBLIC include)
target_link_libraries(app PUBLIC core)
target_link_libraries(app PRIVATE component asset window renderer shader task behavior)
