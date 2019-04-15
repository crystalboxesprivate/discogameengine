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

static glm::vec3 get_forward_from_orientation(const glm::quat &orient) {
  using namespace glm;
  const vec4 forward_vec4 = vec4(0.f, 0.f, 1.f, 1.f);
  const mat4 orientation_matrix = mat4(quat(orient.x, orient.y, orient.z, orient.w));
  vec4 forward_dir_ws = orientation_matrix * forward_vec4;
  // optional normalize
  forward_dir_ws = normalize(forward_dir_ws);
  vec3 forward_vector(forward_dir_ws.x, forward_dir_ws.y, forward_dir_ws.z);
  return forward_vector;
}

} // namespace math
} // namespace utils
