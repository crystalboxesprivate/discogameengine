#pragma once

#include <core/guid.h>
#include <utils/string.h>
#include <core/typedefs.h>

#include <cassert>
#include <functional>
#include <memory>
#include <unordered_map>

#define declare_component(type)                                                                                        \
  template <>                                                                                                          \
  struct component::TypeId<type> {                                                                                     \
    static constexpr u32 id = (u32)utils::string::hash_code(#type);                                                    \
  };

namespace component {
template <typename T>
struct ComponentHandle2 {
  u32 id;
};

template <typename T>
struct TypeId {
  static constexpr u32 id = 0;
};

struct ComponentHandle {
  u32 component_type;
  u32 id;
  ComponentHandle() {
  }
  template <typename T>
  ComponentHandle(ComponentHandle2<T> t_handle) {
    component_type = TypeId<T>::id;
    id = t_handle.id;
  }
};

struct VectorOfComponentsBase {
  virtual ComponentHandle add(i32 entity_id) = 0;
  virtual void remove(ComponentHandle handle) = 0;
};
template <typename T>
struct VectorOfComponents : public VectorOfComponentsBase {

  Vector<T> data;

  struct Slot {
    u32 vector_index = 0;
    u32 entity_id = 0;
  };
  HashMap<u32, Slot> slots;
  Vector<u32> slot_ids;

  inline u32 get_slot_id(u32 entity_id) {
    assert(entity_id < slot_ids.size());
    return slot_ids[entity_id];
  }

  inline T *get(ComponentHandle handle) {
    return slots.find(handle.id) == slots.end() ? nullptr : &data[slots[handle.id].vector_index];
  }

  inline T *get(ComponentHandle2<T> handle) {
    return slots.find(handle.id) == slots.end() ? nullptr : &data[slots[handle.id].vector_index];
  }

  virtual ComponentHandle add(i32 entity_id) {
    assert(slots.find(entity_id) == slots.end());

    slots[entity_id] = Slot();
    slots[entity_id].vector_index = (u32)data.size();
    slots[entity_id].entity_id = entity_id;
    data.push_back(T());
    slot_ids.push_back(entity_id);
    ComponentHandle handle;
    handle.id = entity_id;
    handle.component_type = TypeId<T>::id;
    return handle;
  }

  virtual void remove(ComponentHandle handle) {
    // TODO check if valid.
    i32 target_id = handle.id;
    i32 last_elem_id = slot_ids.back();

    // Get last slot and the slot to be removed.
    Slot &last = slots[last_elem_id];
    Slot &target = slots[target_id];

    last.vector_index = target.vector_index;
    data[target.vector_index] = data[last.vector_index];
    slot_ids[target.vector_index] = last_elem_id;

    data.pop_back();
    slot_ids.pop_back();
    slots.erase(target_id);
  }
};

struct Entity {
  HashMap<u32, ComponentHandle> components;
};

static struct ComponentRegistry *registry_ptr = nullptr;
struct ComponentRegistry {
  ComponentRegistry() {
    assert(!registry_ptr);
    registry_ptr = this;
  }
  HashMap<i32, Entity> entities;
  HashMap<u32, SharedPtr<VectorOfComponentsBase>> components;
};

ComponentRegistry &get_registry(); // {
//  return *registry_ptr;
//}

static u32 allocate_entity() {
  Guid::make_new().a;
  ComponentRegistry &reg = get_registry();
  auto entity_id = Guid::make_new().a;
  reg.entities[entity_id] = Entity();
  return entity_id;
}

static bool delete_entity(u32 entity_id) {
  ComponentRegistry &reg = get_registry();
  Entity &entity = reg.entities[entity_id];
  for (i32 x = 0; x < entity.components.size(); x++) {
    ComponentHandle &handle = entity.components[x];
    reg.components[handle.component_type]->remove(handle);
  }
  return true;
}

template <typename T>
VectorOfComponents<T> &get_vector_of_components() {
  ComponentRegistry &reg = get_registry();
  if (reg.components.find(TypeId<T>::id) == reg.components.end()) {
    reg.components[TypeId<T>::id] = SharedPtr<VectorOfComponentsBase>(new VectorOfComponents<T>);
  }
  return *reinterpret_cast<VectorOfComponents<T> *>(reg.components[TypeId<T>::id].get());
}

template <typename T>
Vector<T> &get_array_of_components() {
  return get_vector_of_components<T>().data;
}

template <typename T>
inline const T *get(ComponentHandle2<T> h) {
  return get_vector_of_components<T>().get(h);
}

template <typename T>
inline T *get_mut(ComponentHandle2<T> h) {
  return get_vector_of_components<T>().get(h);
}

template <typename T>
inline const T *get(ComponentHandle h) {
  return get_vector_of_components<T>().get(h);
}

template <typename T>
inline T *get_mut(ComponentHandle h) {
  return get_vector_of_components<T>().get(h);
}

template <typename T>
ComponentHandle2<T> find(u32 entity_id) {
  auto handle = get_registry().entities[entity_id].components[TypeId<T>::id];
  ComponentHandle2<T> h;
  h.id = handle.id;
  return h;
}

template <typename T>
const T *find_and_get(u32 entity_id) {
  return get<T>(get_registry().entities[entity_id].components[TypeId<T>::id]);
}

template <typename T>
T *find_and_get_mut(u32 entity_id) {
  return get_mut<T>(get_registry().entities[entity_id].components[TypeId<T>::id]);
}

template <typename T>
ComponentHandle2<T> add(u32 entity_id) {
  auto &components = get_vector_of_components<T>();
  auto handle = components.add(entity_id);
  get_registry().entities[entity_id].components[TypeId<T>::id] = handle;
  ComponentHandle2<T> h;
  h.id = handle.id;
  return h;
}

template <typename T>
u32 get_entity_id(i32 component_linear_index) {
  auto &vector_of_components = get_vector_of_components<T>();
  return vector_of_components.slots[vector_of_components.slot_ids[component_linear_index]].entity_id;
}

// Guid allocate_entity();

// ComponentRef get_by_entity(const Guid &entity_id, usize type_id);
// ComponentRef add_to_entity(const Guid &entity_id, usize type_id);
// Vector<ComponentRef> &get_components_of_type_id(usize type_id);

// template <typename T>
// inline Vector<ComponentRef> &get_components_of_type() {
//   return get_components_of_type_id(T::get_type_id());
// }

// template <typename T>
// inline ComponentRef get_shared(const Guid &entity_id) {
//   return get_by_entity(entity_id, T::get_type_id());
// }
// // Returns the pointer to the first component of the T in the vector.
// template <typename T>
// inline T *get(const Guid &entity_id) {
//   return reinterpret_cast<T *>(get_shared<T>(entity_id).get());
// }

// template <typename T>
// inline T &add(const Guid &entity_id) {
//   return *reinterpret_cast<T *>(add_to_entity(entity_id, T::get_type_object()->id).get());
// }

// template <typename T>
// inline ComponentRef add_get_shared(const Guid &entity_id) {
//   return add_to_entity(entity_id, T::get_type_id());
// }

// ComponentRef find(const Guid &component_id);

// // Struct for debugging.
// struct EntityState {
//   EntityState(const Guid &in_entity_id);
//   Guid entity_id;
//   Vector<Component *> components_map;
// };
} // namespace component
// struct ComponentType;
// struct Component {
//   typedef Component Super;
//   inline void set_unique_id(const Guid &in_id) {
//     assert(!id.is_valid());
//     id = in_id;
//   }
//   inline void set_owner(const Guid &in_entity_id) {
//     assert(!entity_id.is_valid());
//     entity_id = in_entity_id;
//   }

//   inline const Guid &get_unique_id() {
//     return id;
//   }
//   inline const Guid &get_owner_id() {
//     return entity_id;
//   }

//   // virtual void serialize_type_id(Archive &archive) = 0;

// private:
//   Guid id;
//   Guid entity_id;
// };
// // typedef std::shared_ptr<Component> ComponentRef;
// typedef Component* ComponentRef;

// template <typename T>
// struct ComponentHandle {
//   ComponentHandle()
//       : ref(nullptr) {
//   }
//   ComponentHandle(ComponentRef in_ref)
//       : ref(in_ref) {
//     if (!ref)
//       return;
//     id = ref->get_unique_id();
//   }
//   Guid id;

//   inline friend Archive &operator<<(Archive &archive, ComponentHandle &reference) {
//     archive << reference.id;
//     return archive;
//   }

//   ComponentHandle &operator=(ComponentRef &b) {
//     ref = b;
//     id = b->get_unique_id();
//     return *this;
//   }

//   ComponentRef get_ref() {
//     if (!id.is_valid())
//       return nullptr;

//     if (!ref)
//       ref = component::find(id);

//     return ref;
//   }

//   inline T *get() {
//     return reinterpret_cast<T *>(get_ref().get());
//   }

// private:
//   ComponentRef ref;
// };

// template <typename T>
// inline T *cast(ComponentRef ref) {
//   return reinterpret_cast<T *>(ref.get());
// }

// template <typename T>
// inline T &get_ref(ComponentRef ref) {
//   return *cast<T>(ref);
// }
// namespace factory {
// Component *make_new(usize component_type_id, const Guid &entity_id, const Guid &guid = Guid());
// }

// struct ComponentFactoryBase {
//   virtual Component *make_new() = 0;
//   static ComponentFactoryBase *factories[128];
//   static usize current_size;
//   usize component_type_id;
// };
// template <typename T>
// struct ComponentFactory : public ComponentFactoryBase {
//   ComponentFactory(usize in_type_id) {
//     constructor_id = current_size;
//     factories[current_size++] = this;
//     component_type_id = in_type_id;
//   }
//   virtual Component *make_new() override {
//     return new T;
//   }
//   usize constructor_id;
// };

// struct ComponentType {
//   ComponentType(usize type_id, ComponentFactoryBase *in_factory)
//       : id(type_id)
//       , factory(in_factory) {
//   }
//   usize id;

// private:
//   ComponentFactoryBase *factory;
// };
// } // namespace component

// Component management
