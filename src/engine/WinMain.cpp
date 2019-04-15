#include <config/config.h>
#include <legacy/sceneloader.h>
#include <utils/path.h>
#include <window/window.h>
#include <app/app.h>
#include <core/log.h>
#include <windows.h>
#include <input/input.h>

#include <task/transform_task.h>
#include <behavior/first_person_controller.h>
#include <behavior/physics.h>

void setup() {
  app::get().add_task(new task::TransformTask);
  app::get().add_behavior(new behavior::FirstPersonCamera);
  app::get().add_behavior(new behavior::PhysicsBehavior);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
  using namespace utils::path;

  logging::Logger logger(join(join(config::CONTENT_DIR, "logs"), "log.txt"));
  using namespace window;
  Window *main_ptr = Window::create("Test", 640, 480, 16, false);
  if (!main_ptr) {
    DEBUG_LOG(System, Error, "Window creation failed");
    return 1;
  }
  Window &main = *main_ptr;
  main.set_cursor_mode(window::CursorMode::Disabled);

  app::App app;
  app.init(main);
  setup();

  auto scenes_folder = join(join(config::CONTENT_DIR, "scenes"), "physics.json");
  legacy::load_json_scene(scenes_folder.c_str());

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
