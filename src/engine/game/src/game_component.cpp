#include <game/debug_component.h>
#include <game/light_component.h>
#include <game/camera_component.h>
#include <game/animation_component.h>
#include <game/metadata_component.h>
#include <game/player_component.h>
#include <game/material_component.h>
#include <game/primitive_component.h>
#include <game/rigid_body_component.h>
#include <game/skinned_mesh_component.h>
#include <game/static_mesh_component.h>

using namespace game;

// void StaticMeshComponent::serialize(Archive &archive) {
//   PrimitiveComponent::serialize(archive);
//   archive << static_mesh;
// }

// void AnimationComponent::serialize(Archive &archive) {
//   Component::serialize(archive);
// }

// void CameraComponent::serialize(Archive &archive) {
//   Super::serialize(archive);
//   archive << speed;
// }

// void LightComponent::serialize(Archive &archive) {
//   Super::serialize(archive);
//   // asse
//   assert(false && "serialization");
//   archive << type;
//   archive << attenuation;
//   archive << diffuse_alpha;
//   archive << is_on;

// }

// void MaterialParameterComponent::serialize(Archive &archive) {
//   Super::serialize(archive);
//   archive << material_diffuse;
//   archive << material_specular;
//   archive << textures;
// }

// void MetadataComponent::serialize(Archive &archive) {
//   Super::serialize(archive);
//   archive << friendly_name;
//   archive << is_updated_by_physics;
// }

// void PlayerComponent::serialize(Archive &archive) {
//   Super::serialize(archive);
//   archive << is_player;
// }

// void PrimitiveComponent::serialize(Archive &archive) {
//   Super::serialize(archive);
//   archive << is_wireframe;
//   archive << is_visible;
//   archive << use_vertex_color;
//   archive << is_unlit;
// }

// void SkinnedMeshComponent::serialize(Archive &archive) {
//   PrimitiveComponent::serialize(archive);
//   assert(false && "Animation serialization is not implemented");
// }

//implement_component(SkinnedMeshComponent);
//implement_component(RigidBodyComponent);
//implement_component(PrimitiveComponent);
//implement_component(PlayerComponent);
//implement_component(MetadataComponent);
//implement_component(MaterialParameterComponent);
//implement_component(LightComponent);
//implement_component(DebugComponent);
//implement_component(CameraComponent);
//implement_component(AnimationComponent);
//implement_component(StaticMeshComponent);
