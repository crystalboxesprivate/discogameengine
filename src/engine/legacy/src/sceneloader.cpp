#include <legacy/sceneloader.h>
#include <utils/path.h>
#include <utils/fs.h>
#include <rapidjson/document.h>
#include <component/component.h>

#include <game/transform_component.h>
#include <game/camera_component.h>
#include <game/metadata_component.h>
#include <game/light_component.h>
#include <game/material_component.h>
#include <game/static_mesh_component.h>
#include <game/rigid_body_component.h>

#include <asset/asset.h>
#include <runtime/texture2d.h>
#include <runtime/static_mesh.h>

#include <config/filesystem.h>

#include <utils/string.h>

using namespace rapidjson;

namespace legacy {
template <typename T>
T get_vec(const Value &arr) {
  T target_vector;
  for (u32 i = 0; i < arr.Size(); i++)
    target_vector[i] = arr[i].GetFloat();
  return target_vector;
}

bool load_json_scene(const char *filename) {
  if (!utils::path::exists(filename)) {
    DEBUG_LOG(System, Error, "Couldn't load scene %s, file not found.", filename);
    return false;
  }
  Document document;
  String json_content = utils::fs::load_file_to_string(filename);

  if (document.Parse<kParseStopWhenDoneFlag>(json_content.c_str()).HasParseError()) {
    DEBUG_LOG(System, Error, "Couldn't pase json file %s", filename);
    return false;
  }

  const char *CAMERA_KEY = "Camera";
  const char *GAMEOBJECTS_KEY = "GameObjects";
  const char *LIGHTS_KEY = "Lights";

  using namespace game;
  using namespace glm;

  if (document.HasMember(CAMERA_KEY)) {
    u32 entity = component::allocate_entity();
    auto camera_handle = component::add<CameraComponent>(entity);
    auto transform_handle = component::add<TransformComponent>(entity);

    auto &camera = *component::get_mut<CameraComponent>(camera_handle);
    auto &transform = *component::get_mut<TransformComponent>(transform_handle);

    const Value &cam = document[CAMERA_KEY];
    camera.speed = cam["Speed"].GetFloat();
    transform.position = get_vec<vec3>(cam["Position"]);
  }

  if (document.HasMember(GAMEOBJECTS_KEY)) {
    const Value &go_js = document[GAMEOBJECTS_KEY];
    Vector<String> static_meshes;

    for (u32 x = 0; x < go_js.Size(); x++) {
      const Value &go = go_js[x];
      if (go.HasMember("SkinnedMesh")) {
        DEBUG_LOG(Assets, Warning, "Got skinned mesh, skipping for now...");
        continue;
      }

      u32 entity = component::allocate_entity();

      auto meta_handle = component::add<MetadataComponent>(entity);
      auto &meta = *component::get_mut<MetadataComponent>(meta_handle);

      meta.friendly_name = go["Name"].GetString();
      meta.is_updated_by_physics = go["Use_Physics"].GetBool();

      auto transform_handle = component::add<TransformComponent>(entity);
      auto &transform = *component::get_mut<TransformComponent>(transform_handle);

      transform.position = get_vec<vec3>(go["Position"]);
      transform.set_mesh_orientation_euler_angles(get_vec<vec3>(go["Rotation"]), true);
      transform.scale3d = get_vec<vec3>(go["Scale"]);

      auto material_handle = component::add<MaterialParameterComponent>(entity);
      auto &material = *component::get_mut<MaterialParameterComponent>(material_handle);
      material.material_diffuse = get_vec<vec4>(go["DiffuseRGB_Alpha"]);
      material.material_specular = get_vec<vec4>(go["SpecularRGB_Alpha"]);

      using namespace utils::path;
      if (go.HasMember("Textures")) {
        auto &textures = go["Textures"];
        for (u32 x = 0; x < textures.Size(); x++) {
          auto &texture_json = textures[x];

          if (texture_json["Strength"].GetFloat() < .5f)
            continue;

          using namespace asset;
          using namespace runtime;
          auto filename = config::CONTENT_DIR + String("/textures/") + texture_json["Name"].GetString();
          AssetRef new_asset = asset::add(filename);

          if (!new_asset) {
            DEBUG_LOG(Assets, Error, "Asset %s is invalid", filename.c_str());
          } else {
            // requests to load data to cpu
            material.textures.push_back(AssetHandle<Texture2D>(new_asset));
          }
        }
      }

      if (go.HasMember("Mesh")) {
        using namespace runtime;
        using namespace asset;
        auto static_mesh_component_handle = component::add<StaticMeshComponent>(entity);
        auto &static_mesh_component = *component::get_mut<StaticMeshComponent>(static_mesh_component_handle);

        auto mesh = go["Mesh"].GetString();
        auto filename = config::CONTENT_DIR + String("/models/") + mesh;

        AssetRef new_asset = asset::add(filename);

        if (!new_asset) {
          DEBUG_LOG(Assets, Error, "Asset %s is invalid", filename.c_str());
        } else {
          static_mesh_component.static_mesh = AssetHandle<StaticMesh>(new_asset);
        }
      }

      if (go.HasMember("RigidBody")) {
        using namespace utils::string;
        using namespace game;
        auto comp_handle = component::add<RigidBodyComponent>(entity);
        auto &comp = *component::get_mut<RigidBodyComponent>(comp_handle);

        const rapidjson::Value &rb = go["RigidBody"];
        auto rigid_body_type = RigidBodyType::Unknown;
        {
          using opt_ptr = usize (*)(const char *);
          constexpr opt_ptr opt = hash_code;
          switch (hash_code(rb["Type"].GetString(), rb["Type"].GetStringLength())) {
          case opt("PLANE"):
            rigid_body_type = RigidBodyType::Plane;
            break;
          case opt("SPHERE"):
            rigid_body_type = RigidBodyType::Sphere;
            break;
          case opt("CYLINDER"):
            rigid_body_type = RigidBodyType::Cylinder;
            break;
          case opt("BOX"):
            rigid_body_type = RigidBodyType::Box;
            break;
          default:
            rigid_body_type = RigidBodyType::Sphere;
            DEBUG_LOG(Physics, Warning, "Undefined type %s, setting up as sphere.", rb["Type"].GetString());
            break;
          }
        }

        comp.type = rigid_body_type;
        if (rb.HasMember("Axis")) {
          auto &axis = rb["Axis"];
          if (axis.IsInt()) {
            comp.axis_id = axis.GetInt();
          } else {
            comp.axis = get_vec<vec3>(axis);
          }
        }

        if (rb.HasMember("HalfExtents")) {
          comp.half_extents = get_vec<vec3>(rb["HalfExtents"]);
        }

        if (rb.HasMember("Pivot")) {
          comp.pivot = get_vec<vec3>(rb["Pivot"]);
        }

        if (rb.HasMember("PivotA")) {
          comp.pivot = get_vec<vec3>(rb["PivotA"]);
        }

        if (rb.HasMember("PivotB")) {
          comp.pivot_b = get_vec<vec3>(rb["PivotB"]);
        }

        if (rb.HasMember("Mass")) {
          comp.mass = rb["Mass"].GetFloat();
        }

        if (rb.HasMember("Offset")) {
          comp.offset = get_vec<vec3>(rb["Offset"]);
        }

        if (rb.HasMember("Radius")) {
          comp.radius = rb["Radius"].GetFloat();
        }

        if (rb.HasMember("Constant")) {
          comp.plane_constant = rb["Constant"].GetFloat();
        }

        if (rb.HasMember("Normal")) {
          comp.plane_normal = get_vec<vec3>(rb["Normal"]);
        }

        comp.transform = component::find<TransformComponent>(entity);
      }
    }
  }

  if (document.HasMember(LIGHTS_KEY)) {
    const Value &go_js = document[LIGHTS_KEY];
    for (u32 x = 0; x < go_js.Size(); x++) {
      const Value &light_json = go_js[x];
      u32 entity = component::allocate_entity();

      auto transform_handle = component::add<TransformComponent>(entity);
    auto &transform = *component::get_mut<TransformComponent>(transform_handle);
      transform.position = get_vec<vec3>(light_json["Position"]);

      auto light_component_handle = component::add<LightComponent>(entity);
      auto &light_component = *component::get_mut<LightComponent>(light_component_handle);
      light_component.attenuation = get_vec<vec4>(light_json["Attenuation"]);
      light_component.diffuse_alpha = get_vec<vec4>(light_json["DiffuseRGB_Alpha"]);
      light_component.is_on = light_json["Turned"].GetString() == String("ON");

      String type_string = light_json["Type"].GetString();
      if (type_string == "POINT_LIGHT")
        light_component.type = LightComponent::LightType::Point;
      if (type_string == "SPOT_LIGHT")
        light_component.type = LightComponent::LightType::Spot;
    }
  }

  return true;
}
} // namespace legacy
