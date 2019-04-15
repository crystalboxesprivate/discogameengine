#include <component/component.h>
#include <app/app.h>
namespace component {

ComponentRegistry &get_registry() {
  return app::get().get_component_registry();
}
// void Component::serialize(Archive &archive) {
//   serialize_type_id(archive);
//   archive << id;
//   archive << entity_id;
// }

// ComponentFactoryBase *ComponentFactoryBase::factories[128];
// usize ComponentFactoryBase::current_size = 0;
// namespace factory {
// Component *make_new(usize component_type_id, const Guid &entity_id, const Guid &guid) {
//   for (int x = 0; x < ComponentFactoryBase::current_size; x++) {
//     if (ComponentFactoryBase::factories[x]->component_type_id != component_type_id) {
//       continue;
//     }
//     Component &component = *ComponentFactoryBase::factories[x]->make_new();
//     component.set_unique_id(guid.is_valid() ? guid : Guid::make_new());
//     component.set_owner(entity_id);
//     return &component;
//   }
//   assert(false);
//   return nullptr;
// }
// } // namespace factory
} // namespace component
