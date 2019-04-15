#pragma once
#include <component/component.h>
namespace game {
using namespace component;
struct AnimationComponent : public Component {
  declare_component(AnimationComponent);
};
} // namespace game
