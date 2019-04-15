add_library(
  game
  STATIC
  include/game/animation_component.h
  include/game/debug_component.h
  include/game/material_component.h
  include/game/metadata_component.h
  include/game/player_component.h
  include/game/primitive_component.h
  include/game/rigid_body_component.h
  include/game/skinned_mesh_component.h
  include/game/static_mesh_component.h
  include/game/transform_component.h
  include/game/light_component.h
  include/game/camera_component.h
  src/game_component.cpp
  src/transform_component.cpp
)
target_include_directories(game PUBLIC include)
target_link_libraries(game PRIVATE runtime core component)
