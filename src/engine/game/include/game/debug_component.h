#pragma once
#include <component/component.h>
namespace game {
using namespace component;
struct DebugComponent {
  bool is_debug_object = false;
};
} // namespace game
declare_component(game::DebugComponent)
