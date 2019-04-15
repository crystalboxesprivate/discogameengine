#pragma once
#include <component/component.h>
namespace game {
using namespace component;
struct DebugComponent /*: public Component*/ {
  bool is_debug_object = false;
};
} // namespace game
declare_component(game::DebugComponent)
