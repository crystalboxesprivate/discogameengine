#include <legacy/sceneloader2.h>
#include <rapidjson/document.h>
#include <utils/fs.h>
#include <utils/path.h>
#include <component/component.h>
#include <game/transform_component.h>
#include <game/metadata_component.h>
#include <game/light_component.h>
#include <game/material_component.h>
#include <game/static_mesh_component.h>
#include <game/rigid_body_component.h>
#include <game/camera_component.h>
#include <asset/asset.h>
#include <runtime/texture2d.h>

using namespace runtime;
using namespace game;
using namespace glm;
using namespace rapidjson;
using namespace asset;

namespace legacy {
String content_dir;

vec3 get_vec3(const Value &js) {
  return vec3(js["x"].GetFloat(), js["y"].GetFloat(), js["z"].GetFloat());
}

quat get_quat(const Value &js) {
  quat q;
  q.x = js["x"].GetFloat();
  q.y = js["y"].GetFloat();
  q.z = js["z"].GetFloat();
  q.w = js["w"].GetFloat();
  return q;
}


vec4 get_color(const Value &js) {
  return vec4(js["red"].GetFloat(), js["green"].GetFloat(), js["blue"].GetFloat(), js["alpha"].GetFloat());
}

String get_path(const String &relative_path) {
  return utils::path::join(content_dir, relative_path);
}

void load(const String &filename) {
  Document document;
  String json_str = utils::fs::load_file_to_string(filename);

  if (document.Parse<kParseStopWhenDoneFlag>(json_str.c_str()).HasParseError()) {
    DEBUG_LOG(System, Error, "Couldn't pase json file %s", filename);
    assert(false);
    return;
  }
  bool is_local_path = document["isLocalPath"].GetBool();
  content_dir = document["contentDirectory"].GetString();

  if (is_local_path) {
    content_dir = utils::path::join(utils::path::parent(filename), content_dir);
	}

  auto &entities = document["entities"];
  for (SizeType x = 0; x < entities.Size(); x++) {
    auto &entity = entities[x]["components"];

    auto entity_id = component::allocate_entity();
    if (entity.HasMember("TransformComponent")) {
      auto &transform_component = entity["TransformComponent"];
      auto &transform = *component::get_mut(component::add<TransformComponent>(entity_id));
      transform.position = get_vec3(transform_component["position"]);
      transform.rotation = get_quat(transform_component["rotation"]);
      transform.scale = get_vec3(transform_component["scale"]);
    }

    if (entity.HasMember("MetadataComponent")) {
      auto &meta_component = entity["MetadataComponent"];
      auto &meta = *component::get_mut(component::add<MetadataComponent>(entity_id));
      meta.name = meta_component["name"].GetString();
    }

    if (entity.HasMember("MaterialComponent")) {
      auto &material_component = entity["MaterialComponent"];
      auto &material = *component::get_mut(component::add<MaterialComponent>(entity_id));
      material.color = get_color(material_component["color"]);
      material.roughness = material_component["roughness"].GetFloat();
      material.metallness = material_component["metal"].GetFloat();

      material.albedo_texture =
          AssetHandle<Texture2D>(asset::add(get_path(material_component["diffuseFilename"].GetString())));
      material.normal_texture =
          AssetHandle<Texture2D>(asset::add(get_path(material_component["normalFilename"].GetString())));
      material.mask_texture =
          AssetHandle<Texture2D>(asset::add(get_path(material_component["occlusionRoughnessMetallic"].GetString())));
    }

    if (entity.HasMember("StaticMeshComponent")) {
      auto &static_mesh_component = entity["StaticMeshComponent"];
      auto &static_mesh = *component::get_mut(component::add<StaticMeshComponent>(entity_id));
      static_mesh.mesh =
          AssetHandle<StaticMesh>(asset::add(get_path(static_mesh_component["filename"].GetString())));
      auto &sm = *static_mesh.mesh.get();
      sm.import_scale_factor = static_mesh_component["importScaleFactor"].GetFloat();
    }

    if (entity.HasMember("RigidBodyComponent")) {
      auto &rigid_body_component = entity["RigidBodyComponent"];
      auto &rigid_body = *component::get_mut(component::add<RigidBodyComponent>(entity_id));
      rigid_body.half_extents = get_vec3(rigid_body_component["extents"]["max"]);
      rigid_body.type = RigidBodyType::Box;
      rigid_body.mass = rigid_body_component["mass"].GetFloat();
    }

    if (entity.HasMember("DirectionalLightComponent")) {
      auto &light_component = entity["DirectionalLightComponent"];
      auto &light = *component::get_mut(component::add<LightComponent>(entity_id));
      light.type = LightType::Directional;
      light.color = get_color(light_component["color"]);
      light.intensity = light_component["intensity"].GetFloat();
    }

    if (entity.HasMember("CameraComponent")) {
      auto &camera_component = entity["CameraComponent"];
      auto &camera = *component::get_mut(component::add<CameraComponent>(entity_id));
      camera.field_of_view = camera_component["fieldOfView"].GetFloat();
      camera.clipping_plane_far = camera_component["clippingPlane"]["far"].GetFloat();
      camera.clipping_plane_near = camera_component["clippingPlane"]["near"].GetFloat();
    }
  }
}
} // namespace legacy