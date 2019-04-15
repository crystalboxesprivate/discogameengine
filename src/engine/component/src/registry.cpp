#include <component/registry.h>

using namespace component;

//void ComponentRegistry::remove_component(const Guid &component_id) {
//  Entry &entry = *helpers::find<Guid, Entry>(component_registry_map, component_id);
//  // remove the record from vectors.
//  {
//    ComponentsVector &vec = *entry.entity_vector.vector_ptr;
//    vec.erase(vec.begin() + entry.entity_vector.index);
//  }
//  {
//    ComponentsVector &vec = *entry.global_vector.vector_ptr;
//    vec.erase(vec.begin() + entry.global_vector.index);
//  }
//  component_registry_map.erase(component_id);
//}
//
//void ComponentRegistry::remove_components_for_entity(const Guid &entity_id) {
//  Vector<Guid> components_to_delete;
//  NameToComponentsVectorMap &comp_map =
//      *get_from_map_safe<EntityIdComponentsMap, Guid, NameToComponentsVectorMap>(
//          entity_id_to_components, entity_id);
//  for (auto it = comp_map.begin(); it != comp_map.end(); it++) {
//    ComponentsVector &components = it->second;
//    for (ComponentRef &ref : components)
//      components_to_delete.push_back(ref->get_unique_id());
//  }
//  entity_id_to_components.erase(entity_id);
//
//  for (Guid &delete_id : components_to_delete) {
//    // Must always find the entry.
//    Entry &entry = *helpers::find<Guid, Entry>(component_registry_map, delete_id);
//    // This must not fail.
//    ComponentsVector &vec = component_type_vectors[entry.type_name];
//    vec.erase(vec.begin() + entry.global_vector.index);
//    component_registry_map.erase(delete_id);
//  }
//}
//
//  ComponentRef ComponentRegistry::find(const Guid &component_id) {
//    Entry* entry= helpers::find<Guid, Entry>(component_registry_map, component_id);
//    if (!entry)
//      return ComponentRef();
//    return entry->component;
//  }
//
//
//ComponentRef ComponentRegistry::get_component(usize type_id, const Guid &entity_id) {
//  NameToComponentsVectorMap &name_to_components_vector_map =
//      *get_from_map_safe<EntityIdComponentsMap, Guid, NameToComponentsVectorMap>(
//          entity_id_to_components, entity_id);
//  ComponentsVector &components_vector =
//      *get_from_map_safe<NameToComponentsVectorMap, usize, ComponentsVector>(
//          name_to_components_vector_map, type_id);
//  return components_vector.size() == 0 ? nullptr : *components_vector.begin();
//}
//
//ComponentRef ComponentRegistry::add_component(usize type_id, const Guid &entity_id,
//                                                   const Guid &deserialized_guid) {
//  ComponentRef new_component =
//      ComponentRef(factory::make_new(type_id, entity_id, deserialized_guid));
//  i32 idx_to_track = -1;
//  ComponentsVector *global_components_vector = nullptr;
//  {
//    // 3.
//    ComponentsVector &components_vector =
//        *get_from_map_safe<NameToComponentsVectorMap, usize, ComponentsVector>(
//            component_type_vectors, type_id);
//    components_vector.push_back(new_component);
//    idx_to_track = (i32)components_vector.size() - 1;
//    global_components_vector = &components_vector;
//  }
//
//  {
//    // 1.
//    NameToComponentsVectorMap &name_to_components_vector_map =
//        *get_from_map_safe<EntityIdComponentsMap, Guid, NameToComponentsVectorMap>(
//            entity_id_to_components, entity_id);
//
//    ComponentsVector &components_vector =
//        *get_from_map_safe<NameToComponentsVectorMap, usize, ComponentsVector>(
//            name_to_components_vector_map, type_id);
//    components_vector.push_back(new_component);
//    // 2.
//    Entry entry;
//    entry.type_name = type_id;
//    entry.component = new_component;
//
//    entry.entity_vector.vector_ptr = &components_vector;
//    entry.entity_vector.index = (i32)components_vector.size() - 1;
//
//    entry.global_vector.vector_ptr = global_components_vector;
//    entry.global_vector.index = idx_to_track;
//    component_registry_map[new_component->get_unique_id()] = entry;
//  }
//
//  return new_component;
//}
