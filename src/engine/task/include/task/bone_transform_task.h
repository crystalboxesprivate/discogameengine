#pragma once

#include <task/task.h>

namespace task {
struct BoneTransformTask : public Task {
  virtual void init() override;
  virtual void update() override;
};
} // namespace task
