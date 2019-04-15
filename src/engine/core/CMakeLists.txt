add_library(
  core
  STATIC
  include/engine
  include/core/typedefs.h
  include/core/archive.h
  include/core/guid.h
  include/core/log.h
  include/core/math/math.h
  include/core/math/color.h
  include/core/math/box.h
  src/archive.cpp
  src/guid.cpp
  src/log.cpp
  src/color.cpp
)
target_include_directories(core PUBLIC include)
target_link_libraries(core PUBLIC glm utils component)
