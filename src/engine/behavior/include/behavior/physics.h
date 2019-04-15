#pragma once

#include <behavior/behavior.h>

namespace behavior {
struct PhysicsWorld;
struct PhysicsFactory;
struct PhysicsBehavior : public Behavior {
  virtual void start() override;
  virtual void update(double delta_time) override;
  virtual void destroy() override;
  PhysicsWorld *world = nullptr;
  PhysicsFactory *factory = nullptr;
};
} // namespace behavior