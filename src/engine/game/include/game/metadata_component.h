#pragma once
#include <component/component.h>
#include <string>

namespace game {
using namespace component;
struct MetadataComponent {
  String friendly_name;
  bool is_updated_by_physics = false;
};
} // namespace game
declare_component(game::MetadataComponent)
