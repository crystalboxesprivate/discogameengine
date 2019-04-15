#pragma once

#include <task/task.h>

namespace task {
  struct TransformTask : public Task {
    virtual void init() override;
    virtual void update() override;
  };
}
