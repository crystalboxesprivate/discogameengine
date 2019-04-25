#pragma once

#include <core/math/math.h>

namespace math {
struct Box {
  glm::vec3 min;
  glm::vec3 max;
  bool valid;

  inline friend Archive &operator<<(Archive &archive, Box &box) {
    archive << box.min;
    archive << box.max;
    archive << box.valid;
    return archive;
  }
};
} // namespace math
