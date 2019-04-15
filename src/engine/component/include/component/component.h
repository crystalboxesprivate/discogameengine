#pragma once

#include <core/guid.h>
#include <utils/string.h>
#include <core/typedefs.h>

#include <cassert>
#include <functional>
#include <memory>
#include <unordered_map>

namespace component {
struct ComponentHandle {
  u32 component_type;
  i32 entity_id;
  i32 id;
  u8 generation;
};

template <typename T>
struct TypeId {
  static constexpr u32 id = 0;
};

struct VectorOfComponentsBase {
  virtual void add() = 0;
  virtual void remove(ComponentHandle handle) = 0;
};
template <typename T>
struct VectorOfComponents : public VectorOfComponentsBase {
  Vector<T> data;

  struct Slot {
    i32 vector_index = 0;
    u8 generation = 0;
  };
  Vector<Slot> slots;
  Vector<i32> pool;

  inline T *get(ComponentHandle handle) {
    return handle.generation != slots[handle.id].generation ? nullptr : &data[slots[handle.id].vector_index];
  }

  virtual ComponentHandle add() {
    // Find available slot id.
    usize slot_id = 0;
    // Pool has zero elements, allocate a new slot.
    if (pool.size() == 0) {
      slots.push_back(Slot());
      slot_id = slots.size() - 1;
    } else {
      // Get slot from the back of the pool. Remove that element from the pool.
      slot_id = pool.back();
      pool.pop_back();
    }
    // Change the generation to invalidate the reference
    slots[slot_id].generation++;
    // Add a new element at the back of the vector.
    slots[slot_id].vector_index = data.size();
    data.push_back(T());
    // Return handle
    return {TypeId<T>::id, -1, slot_id, slots[slot_id].generation};
  }

  virtual void remove(ComponentHandle handle) {
    // TODO check if valid.
    i32 id = handle.id;
    // Get last slot and the slot to be removed.
    Slot &last = slots.back();
    Slot &target = slots[id];
    // Invalidate handles for the slot to be deleted.
    target.generation++;
    // Change the vector index in the swapped slot.
    last.vector_index = target.vector_index;
    // Swap data in the vector with the last element and remove it.
    T copied = data[last.vector_index];
    data[target.vector_index] = copied;
    data.pop_back();
    // Add newly deleted slot to the pool.
    pool.push_back(id);
  }
};

struct Entity {
  Vector<ComponentHandle> components;
};

struct ComponentRegistry {
  HashMap<u32, Entity> entities;
  HashMap<u32, SharedPtr<VectorOfComponentsBase>> components;
};

ComponentRegistry &get_registry();

u32 allocate_entity() {
  Guid::make_new().a;
  ComponentRegistry &reg = get_registry();
  reg.entities[Guid::make_new().a] = Entity();
  return reg.entities.size();
}

bool delete_entity(u32 entity_id) {
  ComponentRegistry &reg = get_registry();
  Entity &entity = reg.entities[entity_id];
  for (i32 x = 0; x < entity.components.size(); x++) {
    ComponentHandle &handle = entity.components[x];
    reg.components[handle.component_type]->remove(handle);
  }
  return true;
}

template <typename T>
VectorOfComponents<T> *get_vector_of_components(u32 type_id) {
  return reinterpret_cast<VectorOfComponents<T> *>(get_registry().components[type_id].get());
}

template <typename T>
Vector<T> &get_array_of_components() {
  return get_vector_of_components(TypeId<T>::id)->data;
}

template <typename T>
inline const T *get(ComponentHandle h) {
  auto components = get_vector_of_components<T>(h.component_type);
  return components->get(h);
}

template <typename T>
ComponentHandle add(u32 entity_id) {
  auto components = get_vector_of_components<T>(TypeId<T>::id);
  auto handle = components->add();
  get_registry().entities[entity_id].push_back(handle);
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
