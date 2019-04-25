#include <config/config.h>
// #include <legacy/sceneloader.h>
#include <utils/path.h>
#include <window/window.h>
#include <app/app.h>
#include <core/log.h>
#include <windows.h>
#include <input/input.h>

#include <task/transform_task.h>
#include <task/bone_transform_task.h>
#include <behavior/first_person_controller.h>
#include <behavior/physics.h>
#include <component/component.h>
#include <game/camera_component.h>
#include <game/rigid_body_component.h>
#include <game/transform_component.h>
#include <game/skinned_mesh_component.h>
#include <game/animation_component.h>

#include <legacy/sceneloader2.h>

#include <renderer/rendercore.h>
#include <shader/cache.h>

#include <runtime/skinned_mesh.h>

void setup() {
  app::get().add_task(new task::TransformTask);
  app::get().add_task(new task::BoneTransformTask);
  app::get().add_behavior(new behavior::FirstPersonCamera);
  app::get().add_behavior(new behavior::PhysicsBehavior);

  app::get().get_shader_cache().default_shader = MaterialShader::add("/Shaders/UserTest.hlslinc").get();
  // app::get().get_shader_cache().default_shader = MaterialShader::add("/Shaders/Default.hlslinc").get();
  app::get().get_shader_cache().compiler.compile();
}

void setup_static_mesh() {
  auto sm_asset = asset::add(utils::path::join(config::CONTENT_DIR, "skinned_mesh/chan.skinnedmeshjson"));

  // Add transform component first
  //asset::load_to_ram(sm_asset, false, true);
  auto entity = component::allocate_entity();
  auto &transform = component::add_and_get<game::TransformComponent>(entity);
  auto &anim = component::add_and_get<game::AnimationComponent>(entity);
  anim.active_animation.name = 3;
  transform.position = glm::vec3(0);
  transform.scale = glm::vec3(0.01f);

  auto &skinned_mesh_component = component::add_and_get<game::SkinnedMeshComponent>(entity);
  skinned_mesh_component.mesh = asset::AssetHandle<runtime::SkinnedMesh>(sm_asset);
}

void post_scene_setup() {
  setup_static_mesh();
  //
  // find camera  attach rigid body set position weight
  u32 entity_id = component::get_entity_id<game::CameraComponent>(0);
  auto &rb = component::add_and_get<game::RigidBodyComponent>(entity_id);
  rb.type = game::RigidBodyType::Sphere;
  rb.position_simulation_weight = (0.f);
  rb.radius = 1.0f;
  rb.mass = 35.f;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
  using namespace utils::path;

  logging::Logger logger(join(join(config::CONTENT_DIR, "logs"), "log.txt"));
  using namespace window;
  Window *main_ptr = Window::create("Test", 1920, 1080, 16, false);
  if (!main_ptr) {
    DEBUG_LOG(System, Error, "Window creation failed");
    return 1;
  }
  Window &main = *main_ptr;
  main.set_cursor_mode(window::CursorMode::Disabled);

  app::App app;
  app.init(main_ptr);
  setup();

  auto scenes_folder = join(join(config::CONTENT_DIR, "scenes"), "teapots.json");
  legacy::load(scenes_folder.c_str());
  post_scene_setup();

  app.game_state_init();

  while (!main.window_should_close()) {
    app.update();
    main.poll_events();
    main.swap_buffers();

    if (input::keyboard::went_up(input::Key::Escape)) {
      main.set_window_should_close(true);
    }
  }

  app.game_state_finalize();

  return 0;
}
