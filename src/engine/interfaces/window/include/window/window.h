#pragma once
#include <engine>
namespace window {
enum class KeyAction : u8 { Released = 0, WentUp = 1, Pressed = 3, WentDown = 7 };

enum class CursorMode { Normal, Hidden, Disabled };
struct Window {
  typedef void (*WindowKeyCallback)(Window *, i32, i32, KeyAction, i32);
  typedef void (*ResizeCallback)(Window *, usize, usize);
  typedef void (*WindowCursorPositionCallback)(Window *, f64, f64);

  virtual bool finalize() = 0;

  virtual double get_time() = 0;

  virtual void get_dimensions(i32 &width, i32 &height) = 0;
  virtual float get_aspect_ratio() = 0;

  virtual void set_key_callback(WindowKeyCallback callback) = 0;
  virtual void set_resize_callback(ResizeCallback callback) = 0;
  virtual void set_mouse_callback(WindowCursorPositionCallback callback) = 0;
  virtual bool window_should_close() = 0;
  virtual void set_window_should_close(bool value) = 0;

  virtual void set_cursor_mode(CursorMode cursor_mode) = 0;
  virtual void set_window_title(const String &new_title) = 0;

  virtual void swap_buffers() = 0;
  virtual void poll_events() = 0;

  virtual void *get_native_window_ptr() = 0;

  // entry
  static Window *create(const char *title_string, i32 width, i32 height, i32 bits, bool fullscreenflag);
};
} // namespace window
