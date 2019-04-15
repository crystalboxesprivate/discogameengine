#pragma once

#include <core/math/math.h>


namespace linear_color {
typedef glm::vec4 LinearColor;
constexpr LinearColor white(1.f, 1.f, 1.f, 1.f);
constexpr LinearColor black(0, 0, 0, 1.f);
constexpr LinearColor red(1.f, 0, 0, 1);
constexpr LinearColor green(0, 1.f, 0, 1);
} // namespace linear_color
