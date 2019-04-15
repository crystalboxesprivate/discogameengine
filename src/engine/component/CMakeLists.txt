add_library(
  component
  STATIC
  include/component/component.h
  include/component/registry.h
  src/component.cpp
  src/registry.cpp
  src/component_actions.cpp
)
target_include_directories(component PUBLIC include)
target_link_libraries(component PUBLIC core)
target_link_libraries(component PRIVATE app)
