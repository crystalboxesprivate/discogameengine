#include <shader/shader.h>
#include <asset/asset.h>
#include <app/app.h>
#include <config/config.h>
#include <core/log.h>
#include <utils/path.h>
#include <graphicsinterface/graphicsinterface.h>
#include <preprocessor/preprocessor.h>
#include <cross/cross.h>
#include <utils/string.h>

#include <renderer/rendercore.h>
#include <shader/cache.h>

int main() {
  logging::Logger logger(utils::path::join(utils::path::join(config::CONTENT_DIR, "logs"), "log.txt"));

  app::App main;
  main.init(nullptr);

  renderer::initialize_default_shaders();
  MaterialShader::add("/Shaders/Default.shader");
  MaterialShader::add("/Shaders/UserTest.shader");
  main.get_shader_cache().compiler.compile();
  // material shader class

  return 0;
}
