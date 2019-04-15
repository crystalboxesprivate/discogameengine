add_library(
  behavior
  STATIC
  include/behavior/behavior.h
  include/behavior/first_person_controller.h
  include/behavior/physics.h
  src/behavior.cpp
  src/first_person_controller.cpp
  src/physics.cpp
)

set(
  bullet_libs 
  bullet3  
  Bullet3Collision
  Bullet3Common
  Bullet3Dynamics
  BulletInverseDynamics
  BulletSoftBody
  LinearMath
)


target_include_directories(behavior PUBLIC include)
target_link_libraries(behavior PUBLIC core)
target_link_libraries(behavior PRIVATE game input app window ${bullet_libs})
