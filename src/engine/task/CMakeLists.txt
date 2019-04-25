add_library(
  task
  STATIC
  include/task/task.h
  include/task/transform_task.h
  include/task/bone_transform_task.h
  src/task.cpp
  src/transform_task.cpp
  src/bone_transform_task.cpp
)
target_include_directories(task PUBLIC include)
target_link_libraries(task PUBLIC core input)
target_link_libraries(task PRIVATE game app window runtime)
