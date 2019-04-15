#pragma once
#include <game/primitive_component.h>
#include <string>
#include <runtime/static_mesh.h>
#include <asset/asset.h>

namespace game {
struct TransformComponent;
struct StaticMeshComponent /*: public PrimitiveComponent*/ {
  //declare_component(StaticMeshComponent);
  StaticMeshComponent()
     /* : PrimitiveComponent() */{
  }
  asset::AssetHandle<runtime::StaticMesh> static_mesh;
  component::ComponentHandle2<TransformComponent> cached_transform_component;
};
} // namespace game
declare_component(game::StaticMeshComponent)
