#pragma once

namespace behavior {
struct Behavior {
  virtual void start() = 0;
  virtual void update(double delta_time) = 0;
  virtual void destroy() {}
};
} // namespace behavior
