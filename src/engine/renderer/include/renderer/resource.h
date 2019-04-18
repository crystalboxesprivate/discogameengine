#pragma once

namespace renderer {
  struct Resource {
    virtual void init() = 0;
    virtual void release() = 0;
  };
} // namespace renderer
