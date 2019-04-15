#pragma once

namespace task {
struct Task {
  virtual void init() = 0;
  virtual void update() = 0;
};
} // namespace  task
