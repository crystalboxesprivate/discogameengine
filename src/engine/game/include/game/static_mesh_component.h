#pragma once
#include <game/primitive_component.h>
#include <string>
#include <runtime/static_mesh.h>
#include <asset/asset.h>

namespace game {
struct TransformComponent;
struct StaticMeshComponent {
  component::ComponentHandle2<TransformComponent> cached_transform_component;
  asset::AssetHandle<runtime::StaticMesh> mesh;
};
} // namespace game
declare_component(game::StaticMeshComponent)
