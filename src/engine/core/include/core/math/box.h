#pragma once

#include <core/math/math.h>

namespace math {
struct Box {
  glm::vec3 min;
  glm::vec3 max;
  bool valid;
};
} // namespace math
