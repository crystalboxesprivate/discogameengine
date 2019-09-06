#include <app/app.h>
#include <asset/registry.h>
#include <component/registry.h>
#include <cassert>
#include <window/window.h>
#include <utils/string.h>
#include <utils/path.h>
#include <utils/fs.h>

#include <renderer/renderer.h>
#include <shader/cache.h>
#include <behavior/behavior.h>
#include <task/task.h>
#include <config/config.h>

using namespace app;
using namespace component;

ComponentRegistry &App::get_component_registry() {
  assert(component_registry);
  return *component_registry.get();
}

asset::Registry &App::get_asset_registry() {
  assert(asset_registry);
  return *asset_registry.get();
}

shader::ShaderCache &App::get_shader_cache() {
  assert(shader_cache);
  return *shader_cache.get();
}

window::Window &App::get_window() {
  return *window_ptr;
}

namespace app {
App *ptr = nullptr;
}

App &app::get() {
  assert(ptr);
  return *ptr;
}

renderer::Renderer &App::get_renderer() {
  return *renderer.get();
}

void App::add_task(task::Task *new_task) {
  tasks.push_back(SharedPtr<task::Task>(new_task));
}

void App::add_behavior(behavior::Behavior *new_behavior) {
  behaviors.push_back(SharedPtr<behavior::Behavior>(new_behavior));
}

void App::init(window::Window *window) {
  // Validate resource integrity?
  if (!utils::path::exists(config::CACHE_DIR)) {
    assert(utils::fs::create_directory(config::CACHE_DIR));
  }
  window_ptr = window;
  // Global pointers should be NULL on initialization.
  assert(!ptr);
  ptr = this;
  assert(!ptr->component_registry);
  assert(!ptr->asset_registry);
  assert(!ptr->shader_cache);
  ptr->component_registry = SharedPtr<ComponentRegistry>(new ComponentRegistry());
  ptr->asset_registry = SharedPtr<asset::Registry>(new asset::Registry());
  ptr->shader_cache = SharedPtr<shader::ShaderCache>(new shader::ShaderCache());
  // Do not create a render instance if the window wasn't initialized.
  if (window_ptr) {
    ptr->renderer = SharedPtr<renderer::Renderer>(new renderer::Renderer());
    renderer->initialize();
  }
}

void App::game_state_init() {
  for (auto &task : tasks) {
    task->init();
  }

  for (auto &behavior : behaviors) {
    behavior->start();
  }
}

void App::game_state_update() {

  for (auto &task : tasks) {
    task->update();
  }

  for (auto &behavior : behaviors) {
    behavior->update(time.delta_seconds);
  }
}

void App::game_state_finalize() {
  for (auto &behavior : behaviors) {
    behavior->destroy();
  }
}

void App::update() {
  using namespace utils;
  window::Window &window = get_window();

  time.update(window.get_time());
  game_state_update();
  renderer->draw_image();

  window.set_window_title(string::sprintf("Game FPS: %d, delta time: %f", time.frames_per_second, time.delta_seconds));
}

void Time::update(const double &new_time) {
  current = new_time;
  delta_seconds = current - previous;

  const double MAX_DELTA_TIME = 0.1;

  if (delta_seconds > MAX_DELTA_TIME) {
    delta_seconds = MAX_DELTA_TIME;
  }
  previous = current;
  frames_since_start++;

  framerate.numframes++;
  if (current - framerate.last_time >= 1.0) {
    frames_per_second = framerate.numframes++;
    framerate.numframes = 0;
    framerate.last_time = current;
  }
}
