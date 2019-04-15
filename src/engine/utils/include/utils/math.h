#include <core/math/math.h>

namespace utils {
namespace math {
// https://stackoverflow.com/questions/9878965/rand-between-0-and-1
template <class T>
T rand01(void) {
  return (T)((double)rand() / (RAND_MAX));
}

// Inspired by:
// https://stackoverflow.com/questions/686353/c-random-float-number-generation
template <class T>
T rand_range(T min, T max) {
  double value = min + static_cast<double>(rand()) / (static_cast<double>(RAND_MAX / (static_cast<double>(max - min))));
  return static_cast<T>(value);
}

glm::quat rotation_between_vectors(glm::vec3 start, glm::vec3 dest);
} // namespace math
} // namespace utils
