#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

#include <cubemap_builder.h>
#include <rendering_utils.h>

#include <cassert>

#define USE_ARGS 1

int main(int argc, char *argv[]) {
#if USE_ARGS
  if (argc < 2)
    return 1;
#endif
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_SAMPLES, 4);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

#ifdef __APPLE__
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

  GLFWwindow *window = glfwCreateWindow(640, 480, "", nullptr, nullptr);
  glfwMakeContextCurrent(window);
  if (window == nullptr) {
    glfwTerminate();
    return -1;
  }

  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    return -1;
  }

  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LEQUAL);
  glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

  const char *filename;
#if USE_ARGS
  assert(argv[1]);
  filename = argv[1];
#else
  filename = "";
#endif
  CubemapBuilder cubemap;
  cubemap.filename = filename;
  cubemap.init();
  cubemap.render_passes();

  // render_brdf_lut(512);
  int screen_width, screen_height;
  glfwGetFramebufferSize(window, &screen_width, &screen_height);
  glViewport(0, 0, screen_width, screen_height);

  glClearColor(0, 0, 0, 0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glfwSwapBuffers(window);
  glfwPollEvents();
  glfwTerminate();
  return 0;
}
