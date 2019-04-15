add_library(
  utils
  STATIC
  include/utils/string.h
  include/utils/path.h
  include/utils/helpers.h
  include/utils/fs.h
  include/utils/math.h
  include/utils/map.h
  src/helpers.cpp
  src/path.cpp
  src/fs.cpp
  src/math.cpp
)

target_include_directories(utils PUBLIC include)
target_link_libraries(utils PUBLIC core)
