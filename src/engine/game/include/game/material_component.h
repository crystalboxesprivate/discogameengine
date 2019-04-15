#pragma once
#include <runtime/texture2d.h>
#include <component/component.h>
namespace game {
using namespace component;
struct MaterialParameterComponent : public Component {
  declare_component(MaterialParameterComponent);
  MaterialParameterComponent()
      : material_diffuse(glm::vec4(1.f, 1.f, 1.f, 1.f))
      , material_specular(glm::vec4(1.f, 1.f, 1.f, 1.f)) {
    // Set random color.
#ifdef ENABLE_TEXTURES
    helpers::rand01<f32>();
    helpers::rand01<f32>();
    material_diffuse[0] = helpers::rand01<f32>();
    material_diffuse[1] = helpers::rand01<f32>();
    material_diffuse[2] = helpers::rand01<f32>();
#endif
  }
  struct TextureInfo {
    TextureInfo()
        : tile_multiplier(1.f) {
    }
    String name;
    i32 cached_texture_id;
    f32 strength;
    f32 tile_multiplier;
  };


  inline void set_diffuse_color(const glm::vec3 &new_diffuse) {
    material_diffuse = glm::vec4(new_diffuse, material_diffuse.a);
  }

  inline void set_alpha_transparency(const f32 &new_alpha) {
    material_diffuse.a = new_alpha;
  }

  inline void set_specular_color(const glm::vec3 &color_rgb) {
    material_specular = glm::vec4(color_rgb, material_specular.a);
  }

  inline void set_specular_power(f32 specPower) {
    material_specular.a = specPower;
  }
  Vector<asset::AssetHandle<runtime::Texture2D>> textures;
  glm::vec4 material_specular; // RGB + specular power
  glm::vec4 material_diffuse;  // RGB + Alpha
};
} // namespace game
