#include <glad/glad.h>

#include <cubemap_builder.h>
#include <rendering_utils.h>

#include <stb/stb_image.h>
#include <stb/stb_image_write.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <shaders.h>

#include <string>
#include <iostream>

class Shader {
public:
  unsigned int id;
  Shader() {
  }
  Shader(const char *vertex_src, const char *fragment_src) {
    unsigned int vertex, fragment;
    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vertex_src, NULL);
    glCompileShader(vertex);
    checkCompileErrors(vertex, "VERTEX");
    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fragment_src, NULL);
    glCompileShader(fragment);
    checkCompileErrors(fragment, "FRAGMENT");
    id = glCreateProgram();
    glAttachShader(id, vertex);
    glAttachShader(id, fragment);
    glLinkProgram(id);
    checkCompileErrors(id, "PROGRAM");
    glDeleteShader(vertex);
    glDeleteShader(fragment);
  }
  void use() {
    glUseProgram(id);
  }
  void set_int(const std::string &name, int value) const {
    glUniform1i(glGetUniformLocation(id, name.c_str()), value);
  }
  void set_float(const std::string &name, float value) const {
    glUniform1f(glGetUniformLocation(id, name.c_str()), value);
  }
  void set_mat4(const std::string &name, const glm::mat4 &mat) const {
    glUniformMatrix4fv(glGetUniformLocation(id, name.c_str()), 1, GL_FALSE, &mat[0][0]);
  }

private:
  void checkCompileErrors(GLuint shader, std::string type) {
    GLint success;
    GLchar info_log[1024];
    if (type != "PROGRAM") {
      glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
      if (!success) {
        glGetShaderInfoLog(shader, 1024, NULL, info_log);
        std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n"
                  << info_log << "\n -- --------------------------------------------------- -- " << std::endl;
        assert(false);
      }
    } else {
      glGetProgramiv(shader, GL_LINK_STATUS, &success);
      if (!success) {
        glGetProgramInfoLog(shader, 1024, NULL, info_log);
        std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n"
                  << info_log << "\n -- --------------------------------------------------- -- " << std::endl;
        assert(false);
      }
    }
  }
};

Shader equirectangular_to_cubemap_shader;
Shader irradiance_shader;
Shader prefilter_shader;

using namespace glm;

mat4 capture_projection = perspective(radians(90.0f), 1.0f, 0.1f, 10.0f);
mat4 capture_views[] = {lookAt(vec3(0.0f, 0.0f, 0.0f), vec3(1.0f, 0.0f, 0.0f), vec3(0.0f, -1.0f, 0.0f)),
                       lookAt(vec3(0.0f, 0.0f, 0.0f), vec3(-1.0f, 0.0f, 0.0f), vec3(0.0f, -1.0f, 0.0f)),
                       lookAt(vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f), vec3(0.0f, 0.0f, 1.0f)),
                       lookAt(vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, -1.0f, 0.0f), vec3(0.0f, 0.0f, -1.0f)),
                       lookAt(vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 1.0f), vec3(0.0f, -1.0f, 0.0f)),
                       lookAt(vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, -1.0f), vec3(0.0f, -1.0f, 0.0f))};

#define PATH FileSystem::getPath

void CubemapBuilder::load_shaders() {
  equirectangular_to_cubemap_shader = Shader(shaders::cubemap_vs, shaders::equirectangular_to_cubemap);
  irradiance_shader = Shader(shaders::cubemap_vs, shaders::irradiance_convolution);
  prefilter_shader = Shader(shaders::cubemap_vs, shaders::prefilter);
}

static const int32_t CUBEMAP_RES = 512;
static const int32_t IRRADIANCE_RES = 32;
static const int32_t PREFILTER_RES = 128;
static const int32_t PREFILTER_MIP_LEVELS = 5;

void CubemapBuilder::init_framebuffer() {

  glGenFramebuffers(1, &capture_fbo);
  glGenRenderbuffers(1, &capture_rbo);

  glBindFramebuffer(GL_FRAMEBUFFER, capture_fbo);
  glBindRenderbuffer(GL_RENDERBUFFER, capture_rbo);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, CUBEMAP_RES, CUBEMAP_RES);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, capture_rbo);
}

void CubemapBuilder::load_hdri() {
  // pbr: load the HDR environment map
  // ---------------------------------
  stbi_set_flip_vertically_on_load(true);
  int32_t nrComponents;
  float *data = stbi_loadf(filename.c_str(), &hdri.width, &hdri.height, &nrComponents, 0);
  if (data) {
    GLuint hdri_id;
    glGenTextures(1, &hdri_id);
    hdri.id = hdri_id;
    glBindTexture(GL_TEXTURE_2D, hdri.id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, hdri.width, hdri.height, 0, GL_RGB, GL_FLOAT,
                 data); // note how we specify the texture's data value to be float

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(data);
  } else {
    std::cout << "Failed to load HDR image." << std::endl;
  }
}

void CubemapBuilder::render_passes() {
  // pbr: setup cubemap to render to and attach to framebuffer
  // ---------------------------------------------------------
  {
    unsigned int envCubemap;
    {
      glGenTextures(1, &envCubemap);
      environment_map = envCubemap;
      glBindTexture(GL_TEXTURE_CUBE_MAP, environment_map);
      for (unsigned int i = 0; i < 6; ++i) {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, CUBEMAP_RES, CUBEMAP_RES, 0, GL_RGB, GL_FLOAT,
                     nullptr);
      }
      glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER,
                      GL_LINEAR_MIPMAP_LINEAR); // enable pre-filter mipmap sampling (combatting
                                                // visible dots artifact)
      glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }

    // pbr: convert HDR equirectangular environment map to cubemap equivalent
    // ----------------------------------------------------------------------
    equirectangular_to_cubemap_shader.use();
    equirectangular_to_cubemap_shader.set_int("equirectangularMap", 0);
    equirectangular_to_cubemap_shader.set_mat4("projection", capture_projection);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, hdri.id);

    glViewport(0, 0, CUBEMAP_RES,
               CUBEMAP_RES); // don't forget to configure the viewport to the capture dimensions.
    glBindFramebuffer(GL_FRAMEBUFFER, capture_fbo);
    for (unsigned int i = 0; i < 6; ++i) {
      equirectangular_to_cubemap_shader.set_mat4("view", capture_views[i]);
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, envCubemap, 0);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      rndering_utils::render_cube();
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
  }
  {
    // then let OpenGL generate mipmaps from first mip face (combatting visible
    // dots artifact)
    glBindTexture(GL_TEXTURE_CUBE_MAP, environment_map);
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

    // pbr: create an irradiance cubemap, and re-scale capture FBO to irradiance
    // scale.
    // --------------------------------------------------------------------------------
    unsigned int irradianceMap;
    glGenTextures(1, &irradianceMap);
    irradiance_map = irradianceMap;
    glBindTexture(GL_TEXTURE_CUBE_MAP, irradiance_map);
    for (unsigned int i = 0; i < 6; ++i) {
      glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, IRRADIANCE_RES, IRRADIANCE_RES, 0, GL_RGB,
                   GL_FLOAT, nullptr);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindFramebuffer(GL_FRAMEBUFFER, capture_fbo);
    glBindRenderbuffer(GL_RENDERBUFFER, capture_rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, IRRADIANCE_RES, IRRADIANCE_RES);

    // pbr: solve diffuse integral by convolution to create an irradiance
    // (cube)map.
    // -----------------------------------------------------------------------------
    irradiance_shader.use();
    irradiance_shader.set_int("environmentMap", 0);
    irradiance_shader.set_mat4("projection", capture_projection);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, environment_map);

    glViewport(0, 0, IRRADIANCE_RES,
               IRRADIANCE_RES); // don't forget to configure the viewport to the capture dimensions.
    glBindFramebuffer(GL_FRAMEBUFFER, capture_fbo);
    for (unsigned int i = 0; i < 6; ++i) {
      irradiance_shader.set_mat4("view", capture_views[i]);
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, irradianceMap,
                             0);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      rndering_utils::render_cube();
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
  }
  {
    // pbr: create a pre-filter cubemap, and re-scale capture FBO to pre-filter
    // scale.
    // --------------------------------------------------------------------------------
    unsigned int prefilterMap;
    glGenTextures(1, &prefilterMap);
    prefilter_map = prefilterMap;
    glBindTexture(GL_TEXTURE_CUBE_MAP, prefilter_map);
    for (unsigned int i = 0; i < 6; ++i) {
      glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, PREFILTER_RES, PREFILTER_RES, 0, GL_RGB, GL_FLOAT,
                   nullptr);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER,
                    GL_LINEAR_MIPMAP_LINEAR); // be sure to set minifcation filter
                                              // to mip_linear
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // generate mipmaps for the cubemap so OpenGL automatically allocates the
    // required memory.
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

    // pbr: run a quasi monte-carlo simulation on the environment lighting to
    // create a prefilter (cube)map.
    // ----------------------------------------------------------------------------------------------------
    prefilter_shader.use();
    prefilter_shader.set_int("environmentMap", 0);
    prefilter_shader.set_mat4("projection", capture_projection);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, environment_map);

    glBindFramebuffer(GL_FRAMEBUFFER, capture_fbo);
    for (uint32_t mip = 0; mip < PREFILTER_MIP_LEVELS; ++mip) {
      // reisze framebuffer according to mip-level size.
      uint32_t mipWidth = (uint32_t)(PREFILTER_RES * std::pow(0.5, mip));
      uint32_t mipHeight = (uint32_t)(PREFILTER_RES * std::pow(0.5, mip));
      glBindRenderbuffer(GL_RENDERBUFFER, capture_rbo);
      glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mipWidth, mipHeight);
      glViewport(0, 0, mipWidth, mipHeight);

      float roughness = (float)mip / (float)(PREFILTER_MIP_LEVELS - 1);
      prefilter_shader.set_float("roughness", roughness);
      for (uint32_t i = 0; i < 6; ++i) {
        prefilter_shader.set_mat4("view", capture_views[i]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, prefilterMap,
                               mip);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        rndering_utils::render_cube();
      }
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
  }

  save_to_disk();
}

#include <vector>
using std::pow;
using std::string;
using std::vector;

// set mode
// bake to hdr
void CubemapBuilder::save_to_disk() {
  struct Image {
    int32_t byte_count = 0;
    int32_t width = 0;
    vector<uint8_t> data;
  };
  // save cubemap
  vector<Image> images;

  for (int x = 0; x < 6; x++) {
    glBindTexture(GL_TEXTURE_CUBE_MAP, environment_map);
    int32_t target = GL_TEXTURE_CUBE_MAP_POSITIVE_X + x;
    int32_t width;
    {
      int32_t ints[10];
      glGetTexLevelParameteriv(target, 0, GL_TEXTURE_WIDTH, ints);
      width = ints[0];
    }
    images.push_back(Image());
    auto &image = images.back();
    image.byte_count = width * width * 12;
    image.width = width;
    image.data.resize(image.byte_count);
    glFlush();
    glGetTexImage(target, 0, GL_RGB, GL_FLOAT, &image.data[0]);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
  }

  for (int x = 0; x < 6; x++) {
    glBindTexture(GL_TEXTURE_CUBE_MAP, irradiance_map);
    int32_t target = GL_TEXTURE_CUBE_MAP_POSITIVE_X + x;
    int32_t width;
    {
      int32_t ints[10];
      glGetTexLevelParameteriv(target, 0, GL_TEXTURE_WIDTH, ints);
      width = ints[0];
    }
    images.push_back(Image());
    auto &image = images.back();
    image.byte_count = width * width * 12;
    image.width = width;
    image.data.resize(image.byte_count);
    glFlush();
    glGetTexImage(target, 0, GL_RGB, GL_FLOAT, &image.data[0]);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
  }

  for (uint32_t mip = 0; mip < PREFILTER_MIP_LEVELS; ++mip) {
    glBindTexture(GL_TEXTURE_CUBE_MAP, prefilter_map);
    for (int x = 0; x < 6; x++) {
      int32_t target = GL_TEXTURE_CUBE_MAP_POSITIVE_X + x;
      int32_t width;
      {
        int32_t ints[10];
        glGetTexLevelParameteriv(target, mip, GL_TEXTURE_WIDTH, ints);
        width = ints[0];
      }
      images.push_back(Image());
      auto &image = images.back();
      image.byte_count = width * width * 12;
      image.width = width;
      image.data.resize(image.byte_count);
      glFlush();
      glGetTexImage(target, mip, GL_RGB, GL_FLOAT, &image.data[0]);
    }
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
  }

  for (int x = 0; x < images.size(); x++) {
    auto &image = images[x];
    string filename = "cubemap_" + std::to_string(x) + ".hdr";
    stbi_write_hdr(filename.c_str(), image.width, image.width, 3, (float *)&image.data[0]);
  }
}

void render_brdf_lut(uint16_t image_size) {

  unsigned int brdf_lut_texture;
  Shader brdfShader(shaders::brdf_vs, shaders::brdf_fs);
  // pbr: setup framebuffer
  // ----------------------
  unsigned int capture_fbo;
  unsigned int capture_rbo;
  glGenFramebuffers(1, &capture_fbo);
  glGenRenderbuffers(1, &capture_rbo);

  glBindFramebuffer(GL_FRAMEBUFFER, capture_fbo);
  glBindRenderbuffer(GL_RENDERBUFFER, capture_rbo);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, image_size, image_size);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, capture_rbo);
  // pbr: generate a 2D LUT from the BRDF equations used.
  // ----------------------------------------------------
  glGenTextures(1, &brdf_lut_texture);

  // pre-allocate enough memory for the LUT texture.
  glBindTexture(GL_TEXTURE_2D, brdf_lut_texture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, image_size, image_size, 0, GL_RGB, GL_FLOAT, 0);
  // be sure to set wrapping mode to GL_CLAMP_TO_EDGE
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  // then re-configure capture framebuffer object and render screen-space quad
  // with BRDF shader.
  glBindFramebuffer(GL_FRAMEBUFFER, capture_fbo);
  glBindRenderbuffer(GL_RENDERBUFFER, capture_rbo);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, image_size, image_size);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, brdf_lut_texture, 0);

  glViewport(0, 0, image_size, image_size);
  brdfShader.use();
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  rndering_utils::render_quad();
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  vector<uint8_t> data(image_size * image_size * 12);
  glBindTexture(GL_TEXTURE_2D, brdf_lut_texture);
  glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_FLOAT, &data[0]);
  glBindTexture(GL_TEXTURE_2D, 0);

  string filename = "brdf_lut.hdr";
  stbi_write_hdr(filename.c_str(), image_size, image_size, 3, (float *)&data[0]);
}
