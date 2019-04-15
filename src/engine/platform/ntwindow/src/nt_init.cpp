#include <windows.h>
#include <ntwindow/ntwindow.h>
#include <graphicsinterface/graphicsinterface.h>
#include <engine>
#include <input/input.h>

// http://nehe.gamedev.net/tutorial/creating_an_opengl_window_(win32)/13001/
// for reference.

static const int NUM_KEYS = 256;
static const int NUM_MOUSE_KEYS = 3;
input::KeyAction keys[NUM_KEYS];
input::KeyAction mouse_keys[NUM_MOUSE_KEYS];
glm::vec2 mouse_pos;
glm::vec2 mouse_delta = glm::vec2(0.f);

void refresh_keys(input::KeyAction *key_buf, u16 num_keys) {
  using input::KeyAction;
  // update keys
  for (int x = 0; x < num_keys; x++) {
    switch (key_buf[x]) {
    case KeyAction::Pressed:
    case KeyAction::Released:
      break;
    case KeyAction::WentDown:
      key_buf[x] = KeyAction::Pressed;
      break;
    case KeyAction::WentUp:
      key_buf[x] = KeyAction::Released;
      break;
    }
  }
}

namespace window {
/*
 * HDC hDC                 Private GDI Device Context
 * HGLRC hRC               Permanent Rendering Context
 * HWND hWnd               Holds Our Window Handle
 * HINSTANCE hInstance     Holds The Instance Of The Application
 *
 * keys[256];              Array Used For The Keyboard Routine
 * active                  Window Active Flag Set To TRUE By Default
 * fullscreen              Fullscreen Flag Set To Fullscreen Mode By Default
 */
HDC hDC = nullptr;
HWND hWnd = nullptr;
HINSTANCE hInstance;

bool active = true;
bool fullscreen = false;
NtWindow *active_window = nullptr;

void update_cursor_mode() {
  assert(active_window);
  switch (active_window->cursor_mode) {
  case CursorMode::Disabled:
  case CursorMode::Hidden:
    ShowCursor(false);
    break;
  default:
    ShowCursor(true);
    break;
  };
}

void kill_nt_window();
void *get_win_ptr() {
  return hWnd;
}

void swap_buffers() {
  SwapBuffers(hDC); // Swap Buffers (Double Buffering)
}

void update_keys() {
  refresh_keys(keys, NUM_KEYS);
  refresh_keys(mouse_keys, NUM_MOUSE_KEYS);
}

/*
 * HWND hWnd,      Handle For This Window
 * UINT uMsg,      Message For This Window
 * WPARAM wParam,  Additional Message Information
 * LPARAM lParam)  Additional Message Information
 */
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
  using input::KeyAction;

  switch (uMsg) {
  case WM_ACTIVATE: {
    if (!HIWORD(wParam)) {
      active = TRUE; // Program Is Active
    } else {
      active = FALSE; // Program Is No Longer Active
    }
    return 0;
  }
    // Intercept System Commands
  case WM_SYSCOMMAND: {
    switch (wParam) {
    case SC_SCREENSAVE:   // Screensaver Trying To Start?
    case SC_MONITORPOWER: // Monitor Trying To Enter Powersave?
      return 0;           // Prevent From Happening
    }
    break; // Exit
  }

  case WM_CLOSE: {
    active_window->is_closing = true;
    PostQuitMessage(0); // Send A Quit Message
    return 0;           // Jump Back
  }

  case WM_MOUSEMOVE: {

    POINT point;
    GetCursorPos(&point);
    mouse_pos = glm::vec2(point.x, point.y);
    if (active_window->cursor_mode == CursorMode::Disabled) {
      mouse_pos += mouse_delta;

      i32 w, h;
      active_window->get_dimensions(w, h);

      glm::vec2 half_window(w / 2.f, h / 2.f);

      SetCursorPos((i32)half_window.x, (i32)half_window.y);
      mouse_delta += glm::vec2(half_window.x - point.x, half_window.y - point.y);
    }
    return 0;
  }

  case WM_KEYDOWN: {
    keys[wParam] = KeyAction::WentDown;

    if (active_window->key_callback) {
      active_window->key_callback(active_window, (i32)wParam, 0, window::KeyAction::WentDown, 0);
    }
    return 0;
  }
  case WM_KEYUP: {
    keys[wParam] = KeyAction::WentUp;

    if (active_window->key_callback) {
      active_window->key_callback(active_window, (i32)wParam, 0, window::KeyAction::WentUp, 0);
    }
    return 0;
  }

  case WM_MBUTTONDOWN: {
    mouse_keys[(u8)input::MouseButton::Middle] = KeyAction::WentDown;
    return 0;
  }

  case WM_MBUTTONUP: {
    mouse_keys[(u8)input::MouseButton::Middle] = KeyAction::WentUp;
    return 0;
  }

  case WM_RBUTTONDOWN: {
    mouse_keys[(u8)input::MouseButton::Right] = KeyAction::WentDown;
    return 0;
  }

  case WM_RBUTTONUP: {
    mouse_keys[(u8)input::MouseButton::Right] = KeyAction::WentUp;
    return 0;
  }

  case WM_LBUTTONDOWN: {
    mouse_keys[(u8)input::MouseButton::Left] = KeyAction::WentDown;
    return 0;
  }

  case WM_LBUTTONUP: {
    mouse_keys[(u8)input::MouseButton::Left] = KeyAction::WentUp;
    return 0;
  }

  case WM_SIZE: {
    if (active_window->resize_callback) {
      usize width = LOWORD(lParam);
      usize height = HIWORD(lParam);
      if (height == 0) {
        height = 1; // Making Height Equal One
      }
      active_window->resize_callback(active_window, width, height);
    }

    return 0;
  }
  }

  return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

Window *Window::create(const char *title, i32 width, i32 height, i32 bits, bool fullscreenflag) {
  NtWindow &window = *new NtWindow();
  active_window = &window;

  u32 PixelFormat;                  // Holds The Results After Searching For A Match
  WNDCLASS wc;                      // Windows Class Structure
  DWORD dwExStyle;                  // Window Extended Style
  DWORD dwStyle;                    // Window Style
  RECT WindowRect;                  // Grabs Rectangle Upper Left / Lower Right Values
  WindowRect.left = (long)0;        // Set Left Value To 0
  WindowRect.right = (long)width;   // Set Right Value To Requested Width
  WindowRect.top = (long)0;         // Set Top Value To 0
  WindowRect.bottom = (long)height; // Set Bottom Value To Requested Height
                                    // Set The Global Fullscreen Flag
  fullscreen = fullscreenflag;

  hInstance = GetModuleHandle(NULL);             // Grab An Instance For Our Window
  wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC; // Redraw On Size, And Own DC For Window.
  wc.lpfnWndProc = (WNDPROC)WndProc;             // WndProc Handles Messages
  wc.cbClsExtra = 0;                             // No Extra Window Data
  wc.cbWndExtra = 0;                             // No Extra Window Data
  wc.hInstance = hInstance;                      // Set The Instance
  wc.hIcon = LoadIcon(NULL, IDI_WINLOGO);        // Load The Default Icon
  wc.hCursor = LoadCursor(NULL, IDC_ARROW);      // Load The Arrow Pointer
  wc.hbrBackground = NULL;                       // No Background Required For GL
  wc.lpszMenuName = NULL;                        // We Don't Want A Menu
  wc.lpszClassName = "disco_engine";             // Set The Class Name
                                                 // Attempt To Register The Window Class
  if (!RegisterClass(&wc)) {
    DEBUG_LOG(System, Error, "Failed To Register The Window Class.");
    return nullptr; // Return FALSE
  }

  // Attempt Fullscreen Mode?
  if (fullscreen) {
    DEVMODE dmScreenSettings;                               // Device Mode
    memset(&dmScreenSettings, 0, sizeof(dmScreenSettings)); // Makes Sure Memory's Cleared
    dmScreenSettings.dmSize = sizeof(dmScreenSettings);     // Size Of The Devmode Structure
    dmScreenSettings.dmPelsWidth = width;                   // Selected Screen Width
    dmScreenSettings.dmPelsHeight = height;                 // Selected Screen Height
    dmScreenSettings.dmBitsPerPel = bits;                   // Selected Bits Per Pixel
    dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

    // Try To Set Selected Mode And Get Results.  NOTE: CDS_FULLSCREEN Gets Rid Of Start Bar.
    if (ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL) {
      // If The Mode Fails, Offer Two Options.  Quit Or Use Windowed Mode.
      DEBUG_LOG(System, Error, "The Requested Fullscreen Mode Is Not Supported By. Going back to windowed.?");
      fullscreen = false;
    }
  }
  // Are We Still In Fullscreen Mode?
  if (fullscreen) {
    dwExStyle = WS_EX_APPWINDOW; // Window Extended Style
    dwStyle = WS_POPUP;          // Windows Style
    ShowCursor(FALSE);           // Hide Mouse Pointer
  } else {
    dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE; // Window Extended Style
    dwStyle = WS_OVERLAPPEDWINDOW;                  // Windows Style
  }
  // Adjust Window To True Requested Size
  AdjustWindowRectEx(&WindowRect, dwStyle, FALSE, dwExStyle);

  // Create The Window
  if (!(hWnd = CreateWindowEx(dwExStyle,                          // Extended Style For The Window
                              "disco_engine",                     // Class Name
                              title,                              // Window Title
                              dwStyle |                           // Defined Window Style
                                  WS_CLIPSIBLINGS |               // Required Window Style
                                  WS_CLIPCHILDREN,                // Required Window Style
                              0, 0,                               // Window Position
                              WindowRect.right - WindowRect.left, // Calculate Window Width
                              WindowRect.bottom - WindowRect.top, // Calculate Window Height
                              NULL,                               // No Parent Window
                              NULL,                               // No Menu
                              hInstance,                          // Instance
                              NULL)))                             // Dont Pass Anything To WM_CREATE
  {
    kill_nt_window(); // Reset The Display
    volatile auto err = GetLastError();
    DEBUG_LOG(System, Error, "Window Creation Error: Error code %d", err); //, "ERROR", MB_OK | MB_ICONEXCLAMATION);
    return nullptr;                                                        // Return FALSE
  }
  // pfd Tells Windows How We Want Things To Be
  static PIXELFORMATDESCRIPTOR pfd = {
      sizeof(PIXELFORMATDESCRIPTOR), // Size Of This Pixel Format Descriptor
      1,                             // Version Number
      PFD_DRAW_TO_WINDOW |           // Format Must Support Window
          PFD_SUPPORT_OPENGL |       // Format Must Support OpenGL
          PFD_DOUBLEBUFFER,          // Must Support Double Buffering
      PFD_TYPE_RGBA,                 // Request An RGBA Format
      (u8)bits,                      // Select Our Color Depth
      0,
      0,
      0,
      0,
      0,
      0, // Color Bits Ignored
      0, // No Alpha Buffer
      0, // Shift Bit Ignored
      0, // No Accumulation Buffer
      0,
      0,
      0,
      0,              // Accumulation Bits Ignored
      16,             // 16Bit Z-Buffer (Depth Buffer)
      0,              // No Stencil Buffer
      0,              // No Auxiliary Buffer
      PFD_MAIN_PLANE, // Main Drawing Layer
      0,              // Reserved
      0,
      0,
      0 // Layer Masks Ignored
  };

  // Did We Get A Device Context?
  if (!(hDC = GetDC(hWnd))) {
    kill_nt_window(); // Reset The Display
    DEBUG_LOG(System, Error, "Can't Create A Device Context.");
    return nullptr; // Return FALSE
  }
  // Did Windows Find A Matching Pixel Format?
  if (!(PixelFormat = ChoosePixelFormat(hDC, &pfd))) {
    kill_nt_window(); // Reset The Display
    DEBUG_LOG(System, Error, "Can't Find A Suitable PixelFormat.");
    return nullptr; // Return FALSE
  }
  // Are We Able To Set The Pixel Format?
  if (!SetPixelFormat(hDC, PixelFormat, &pfd)) {
    kill_nt_window(); // Reset The Display
    DEBUG_LOG(System, Error, "Can't Set The PixelFormat.");
    return nullptr; // Return FALSE
  }
  // create hardware interface here...

  if (!graphicsinterface::create_context(&window, hDC)) {
    kill_nt_window();
    return nullptr;
  }

  ShowWindow(hWnd, SW_SHOW); // Show The Window
  SetForegroundWindow(hWnd); // Slightly Higher Priority
  SetFocus(hWnd);            // Sets Keyboard Focus To The Window

  window.timer.reset();
  window.timer.start();

  // ShowCursor(false);

  return &window;
}

// Resize And Initialize The GL Window
void kill_nt_window() {
  // Are We In Fullscreen Mode?
  if (fullscreen) {
    ChangeDisplaySettings(NULL, 0); // If So Switch Back To The Desktop
    ShowCursor(TRUE);               // Show Mouse Pointer
  }
  // Do We Have A Rendering Context?
  if (graphicsinterface::get_native_context()) {
    graphicsinterface::delete_context();
  }
  // Are We Able To Release The DC
  if (hDC && !ReleaseDC(hWnd, hDC)) {
    DEBUG_LOG(System, Error, "Release Device Context Failed.");
    hDC = NULL; // Set DC To NULL
  }
  // Are We Able To Destroy The Window?
  if (hWnd && !DestroyWindow(hWnd)) {
    DEBUG_LOG(System, Error, "Could Not Release hWnd.");
    hWnd = NULL; // Set hWnd To NULL
  }
  // Are We Able To Unregister Class
  if (!UnregisterClass("OpenGL", hInstance)) {
    DEBUG_LOG(System, Error, "Could Not Unregister Class.");
    hInstance = NULL; // Set hInstance To NULL
  }
  delete active_window;
  active_window = nullptr;
}
} // namespace window

// Input implementation
namespace input {
namespace keyboard {
bool is_pressed(Key key) {
  return keys[(i16)key] == KeyAction::Pressed;
}
bool went_down(Key key) {
  return keys[(i16)key] == KeyAction::WentDown;
}
bool went_up(Key key) {
  return keys[(i16)key] == KeyAction::WentUp;
}
} // namespace keyboard

namespace mouse {
glm::vec2 get_position() {
  return mouse_pos;
}
bool is_pressed(MouseButton key) {
  return mouse_keys[(u8)key] == KeyAction::Pressed;
}
bool went_down(MouseButton key) {
  return mouse_keys[(u8)key] == KeyAction::WentDown;
}
bool went_up(MouseButton key) {
  return mouse_keys[(u8)key] == KeyAction::WentUp;
}
} // namespace mouse

namespace gamepad {}
} // namespace input