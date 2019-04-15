#pragma once
#include <component/component.h>
#include <string>

namespace game {
using namespace component;
struct MetadataComponent : public Component {
  declare_component(MetadataComponent);
  MetadataComponent() : is_updated_by_physics(false) {}
  String friendly_name;
  bool is_updated_by_physics;

};
} // namespace components
