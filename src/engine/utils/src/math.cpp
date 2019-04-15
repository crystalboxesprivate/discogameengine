#include <utils/math.h>

namespace utils {
namespace math {
using namespace glm;
quat rotation_between_vectors(vec3 start, vec3 dest) {
  start = normalize(start);
  dest = normalize(dest);

  float cos_theta = dot(start, dest);
  vec3 rotation_axis;

  if (cos_theta < -1 + 0.001f) {
    // special case when vectors in opposite directions:
    // there is no "ideal" rotation axis
    // So guess one; any will do as long as it's perpendicular to start
    rotation_axis = cross(vec3(0.0f, 0.0f, 1.0f), start);
    // bad luck, they were parallel, try again!
    if (length2(rotation_axis) < 0.01)
      rotation_axis = cross(vec3(1.0f, 0.0f, 0.0f), start);

    rotation_axis = normalize(rotation_axis);
    return angleAxis(radians(180.0f), rotation_axis);
  }
  rotation_axis = cross(start, dest);
  float square_root_value = sqrt((1 + cos_theta) * 2);
  float invs = 1 / square_root_value;

  return quat(square_root_value * 0.5f, rotation_axis.x * invs, rotation_axis.y * invs, rotation_axis.z * invs);
}
} // namespace math
} // namespace core