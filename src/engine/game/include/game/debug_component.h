#pragma once
#include <component/component.h>
namespace game {
using namespace component;
struct DebugComponent : public Component {
  declare_component(DebugComponent);
  DebugComponent() : is_debug_object(false) {
  }
  // TODO: check if this one is actually needed
  bool is_debug_object;
};
} // namespace game
