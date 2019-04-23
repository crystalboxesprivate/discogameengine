#pragma once
#include <component/component.h>
#include <string>

namespace game {
using namespace component;
struct MetadataComponent {
  String name;
};
} // namespace game
declare_component(game::MetadataComponent)
