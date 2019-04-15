#include <component/component.h>
#include <component/registry.h>
#include <app/app.h>

namespace component {
Guid allocate_entity() {
  Guid guid = Guid::make_new();
  auto &registry = app::get().get_component_registry();
  registry.entity_id_to_components[guid] = ComponentRegistry::NameToComponentsVectorMap();
  return guid;
}

ComponentRef find(const Guid &component_id) {
  return app::get().get_component_registry().find(component_id);
}

ComponentRef get_by_entity(const Guid &entity_id, usize type_id) {
  return app::get().get_component_registry().get_component(type_id, entity_id);
}
ComponentRef add_to_entity(const Guid &entity_id, usize type_id) {
  return app::get().get_component_registry().add_component(type_id, entity_id);
}
EntityState::EntityState(const Guid &in_entity_id)
    : entity_id(in_entity_id) {
  auto &map = app::get().get_component_registry().entity_id_to_components[in_entity_id];
  for (auto &it : map)
    for (auto &vec_item : it.second)
      components_map.push_back(vec_item.get());
}
Vector<ComponentRef> &get_components_of_type_id(usize type_id) {
  return app::get().get_component_registry().get_components_of_type(type_id);
}
} // namespace component
