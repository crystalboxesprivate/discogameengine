#pragma once
#include <runtime/texture2d.h>
#include <component/component.h>
namespace game {
using namespace component;
struct MaterialComponent {
  glm::vec4 color = glm::vec4(.8f, .8f, .8f, 1.f);
  float roughness = 1.f;
  float metallness = 1.f;
  asset::AssetHandle<runtime::Texture2D> albedo_texture;
  asset::AssetHandle<runtime::Texture2D> normal_texture;
  asset::AssetHandle<runtime::Texture2D> mask_texture;

  Vector<asset::AssetHandle<runtime::Texture2D>> textures;
  glm::vec4 material_specular = glm::vec4(1.f, 1.f, 1.f, 1.f); // RGB + specular power
  glm::vec4 material_diffuse = glm::vec4(1.f, 1.f, 1.f, 1.f);  // RGB + Alpha
};
} // namespace game
declare_component(game::MaterialComponent)
