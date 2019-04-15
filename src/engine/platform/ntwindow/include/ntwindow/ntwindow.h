#pragma once
#include <window/window.h>
#include <ntwindow/game_timer.h>

namespace window {
struct NtWindow : public Window {
  virtual bool finalize() override;

  virtual double get_time() override;

  virtual void set_key_callback(Window::WindowKeyCallback callback) override;
  virtual void set_resize_callback(ResizeCallback callback) override;
  virtual void set_mouse_callback(Window::WindowCursorPositionCallback callback) override;
  virtual bool window_should_close() override;
  virtual void set_window_should_close(bool value) override;
  virtual void set_window_title(const String &new_title) override;

  virtual void swap_buffers() override;
  virtual void poll_events() override;

  virtual void *get_native_window_ptr() override;
  
  virtual void get_dimensions(i32 &width, i32 &height) override;
  virtual float get_aspect_ratio() override;

  virtual void set_cursor_mode(CursorMode cursor_mode) override;

  bool is_closing = false;
  Window::WindowKeyCallback key_callback = nullptr;
  Window::ResizeCallback resize_callback = nullptr;
  Window::WindowCursorPositionCallback cursor_pos_callback = nullptr;

  GameTimer timer;
  CursorMode cursor_mode = CursorMode::Normal;
};
} // namespace window
