#pragma once
#include <component/component.h>
#include <string>

namespace game {
using namespace component;
struct PlayerComponent : public Component {
  declare_component(PlayerComponent);
  PlayerComponent() : is_player(false) {
  }
  bool is_player;
};
} // namespace component
