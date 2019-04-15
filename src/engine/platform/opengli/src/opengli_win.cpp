#include <opengli/opengli.h>
#include <engine>
#include <windows.h>
#include <gl/gl.h>
#include <gl/glu.h>

#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glu32.lib")
#pragma comment(lib, "odbc32.lib")
#pragma comment(lib, "odbccp32.lib")

namespace graphicsinterface {
HGLRC hRC = nullptr;

void *get_native_device() {
  return nullptr;
}
void *get_native_context() {
  return hRC;
}

bool create_context(void *window_ptr, void *data) {
  HDC hDC = reinterpret_cast<HDC>(data);
  if (!(hRC = wglCreateContext(hDC))) {
    DEBUG_LOG(System, Error, "Can't Create A GL Rendering Context.");
    return false;
  }

  if (!wglMakeCurrent(hDC, hRC)) {
    DEBUG_LOG(System, Error, "Can't Activate The GL Rendering Context.");
    return false;
  }

  DEBUG_LOG(Rendering, Log, "Initialized OpenGL context: %s", (char *)glGetString(GL_VERSION));
  return true;
}

bool delete_context() {
  if (!wglMakeCurrent(NULL, NULL)) {
    DEBUG_LOG(System, Error, "Release Of DC And RC Failed.");
  }
  if (!wglDeleteContext(hRC)) {
    DEBUG_LOG(System, Error, "Release Rendering Context Failed.");
  }
  hRC = nullptr;
  return true;
}
} // namespace graphicsinterface
