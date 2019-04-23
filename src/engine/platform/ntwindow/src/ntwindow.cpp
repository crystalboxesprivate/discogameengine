#include <ntwindow/ntwindow.h>
#include <windows.h>

namespace window {
extern HWND hWnd;

void kill_nt_window();
void swap_buffers();
void *get_win_ptr();
void update_keys();
void update_cursor_mode();

bool NtWindow::finalize() {
  kill_nt_window();
  return true;
}

float NtWindow::get_aspect_ratio() {
  i32 w, h;
  get_dimensions(w, h);
  return w / (float)h;
}

void NtWindow::set_window_should_close(bool value) {
  is_closing = value;
}

void NtWindow::get_dimensions(i32 &width, i32 &height) {
  RECT rect;
  if (GetWindowRect(hWnd, &rect)) {
    width = rect.right - rect.left;
    height = rect.bottom - rect.top;
  }
}

void NtWindow::set_window_title(const String &new_title) {
  SetWindowText(hWnd, new_title.c_str());
}

double NtWindow::get_time() {
  return max(0.f, timer.get_total_time());
}

void NtWindow::set_resize_callback(ResizeCallback callback) {
  resize_callback = callback;
}

void NtWindow::set_key_callback(Window::WindowKeyCallback callback) {
  key_callback = callback;
}

void NtWindow::set_mouse_callback(Window::WindowCursorPositionCallback callback) {
  cursor_pos_callback = callback;
}

void NtWindow::set_cursor_mode(CursorMode in_cursor_mode) {
  cursor_mode = in_cursor_mode;
  update_cursor_mode();
}

bool NtWindow::window_should_close() {
  return is_closing;
}

void NtWindow::swap_buffers() {
  timer.tick();
  window::swap_buffers();
}

void NtWindow::poll_events() {
  update_keys();
  MSG msg;
  if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
    if (msg.message == WM_QUIT) {
      is_closing = true;
    } else {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
  }
}

void *NtWindow::get_native_window_ptr() {
  return get_win_ptr();
}

} // namespace window
