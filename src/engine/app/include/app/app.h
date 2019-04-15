#pragma once
#include <engine>

namespace window {
struct Window;
}

namespace task {
struct Task;
}

namespace behavior {
struct Behavior;
}

namespace component {
struct ComponentRegistry;
} // namespace component

namespace shader {
struct ShaderCache;
} // namespace shader

namespace renderer {
struct Renderer;
} // namespace renderer

namespace asset {
struct Registry;
} // namespace asset

namespace app {
struct Time {
  struct FrameRate {
    double last_time = 0;
    int numframes = 0;
  };

  double current = 0;
  double previous = 0;
  double delta_seconds = 0;
  int frames_since_start = 0;
  int frames_per_second = 0;
  FrameRate framerate;

  void update(const double &new_time);
};

struct App {
  void init(window::Window &window);
  void update();
  component::ComponentRegistry &get_component_registry();
  shader::ShaderCache &get_shader_cache();
  asset::Registry &get_asset_registry();
  renderer::Renderer &get_renderer();
  window::Window &get_window();
  Time time;

  void add_task(task::Task *new_task);
  void add_behavior(behavior::Behavior *new_behavior);

  void game_state_init();
  void game_state_finalize();

private:
  void game_state_update();

  SharedPtr<component::ComponentRegistry> component_registry;
  SharedPtr<renderer::Renderer> renderer;
  SharedPtr<asset::Registry> asset_registry;
  SharedPtr<shader::ShaderCache> shader_cache;
  window::Window *window_ptr;

  Vector<SharedPtr<task::Task>> tasks;
  Vector<SharedPtr<behavior::Behavior>> behaviors;
};

App &get();
} // namespace app
