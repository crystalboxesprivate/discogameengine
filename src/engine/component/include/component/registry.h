#pragma once

#include <component/component.h>
#include <core/guid.h>
#include <utils/helpers.h>
#include <list>

namespace component {


// struct ComponentRegistry {
//   Vector<Entity> entities;
//   HashMap<u32, SharedPtr<VectorOfComponentsBase>> components;


  // // 1. Entity id -> component type -> vector of component refs
  // typedef Vector<ComponentRef> ComponentsVector;
  // typedef HashMap<usize, ComponentsVector> NameToComponentsVectorMap;
  // typedef HashMap<Guid, NameToComponentsVectorMap> EntityIdComponentsMap;

  // struct VectorEntry {
  //   i32 index;
  //   ComponentsVector *vector_ptr;
  // };

  // struct Entry {
  //   // Type identifier. Must be always static.
  //   usize type_name;
  //   ComponentRef component;
  //   VectorEntry entity_vector;
  //   VectorEntry global_vector;
  // };

  // // 3. Component type -> vector of components. Makes it easy to iterate.
  // EntityIdComponentsMap entity_id_to_components;
  // HashMap<Guid, Entry> component_registry_map;
  // HashMap<usize, SharedPtr<VectorOfComponentsBase>> component_type_vectors;

  // template <typename M, typename K, typename V>
  // inline V *get_from_map_safe(M &map, const K &key) {
  //   V *value = helpers::find<K, V>(map, key);
  //   if (!value) {
  //     map[key] = V();
  //     return get_from_map_safe<M, K, V>(map, key);
  //   }
  //   return value;
  // }

  // template <typename T>
  // inline usize get_type_id() {
  //   return T::get_type_id();
  // }

  // // Use only this method to spawn new components to the world.
  // template <typename T>
  // inline ComponentRef add_component(const Guid &entity_id, const Guid &deserialized_guid = Guid()) {
  //   add_component(get_type_id<T>(), entity_id, deserialized_guid);
  // }
  // ComponentRef find(const Guid &component_id);
  // ComponentRef get_component(usize type_id, const Guid &entity_id);
  // ComponentRef add_component(usize type_id, const Guid &entity_id, const Guid &deserialized_guid = Guid());
  // // Handy method when the entity is removed.
  // void remove_components_for_entity(const Guid &entity_id);
  // // This will crash as soon as the invalid id will be provided.
  // void remove_component(const Guid &component_id);

  // inline ComponentsVector &get_components_of_type(usize type_id) {
  //   return *get_from_map_safe<NameToComponentsVectorMap, usize, ComponentsVector>(component_type_vectors, type_id);
  // }
// };
} // namespace component
