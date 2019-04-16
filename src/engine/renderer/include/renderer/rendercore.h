#pragma once

#include <shader/shader.h>
#include <asset/factory.h>
#include <preprocessor/preprocessor.h>

namespace renderer {

void initialize_default_shaders();
} // namespace renderer

#define declare_vertex_type(type)                                                                                      \
  constexpr u32 guid() {                                                                                               \
    return (u32)utils::string::hash_code(#type);                                                                       \
  }                                                                                                                    \
  virtual u32 get_unique_id() override {                                                                               \
    return guid();                                                                                                     \
  }                                                                                                                    \
  virtual const char *get_filename() override;                                                                         \
  static VertexTypeImplBase *get_vertex_impl();

#define implement_vertex_type(type, filename)                                                                          \
  type g_type##type;                                                                                                   \
  VertexTypeImpl<type> g_##type(&g_type##type);                                                                        \
  const char *type::get_filename() {                                                                                   \
    return filename;                                                                                                   \
  }                                                                                                                    \
  VertexTypeImplBase *type::get_vertex_impl() {                                                                        \
    return &g_##type;                                                                                                  \
  }

struct VertexType {
  virtual u32 get_unique_id() = 0;
  virtual const char *get_filename() = 0;

  void init() {
  }
};

struct VertexTypeImplBase {};
template <typename T>
struct VertexTypeImpl : public VertexTypeImplBase {
  VertexTypeImpl(VertexType *vert_type) {
    vertex_types.push_back(vert_type);
  }
};

struct StaticMeshVertexType : VertexType {
  declare_vertex_type(StaticMeshVertexType)
};

struct SkinnedMeshVertexType : VertexType {
  declare_vertex_type(SkinnedMeshVertexType)
};

struct RenderPass {
  virtual void get_compile_jobs(Vector<shader::compiler::Input> &inputs) = 0;
  virtual const char *get_filename() = 0;
  static void preprocess_vertex_type(const String &filename, shader::compiler::Environment &env);

  struct ShaderId {
    Guid global_id;
    shader::ShaderRef Shader;

    inline friend Archive &operator<<(Archive &archive, ShaderId &id) {
      archive << id.global_id;
      return archive;
    }
  };
  struct ShaderPair {
    ShaderId vertex;
    ShaderId pixel;

    inline friend Archive &operator<<(Archive &archive, ShaderPair &pair) {
      archive << pair.vertex;
      archive << pair.pixel;
      return archive;
    }
  };
};

struct RenderPassImplBase {};
template <typename T>
struct RenderPassImpl : public RenderPassImplBase {
  RenderPassImpl(RenderPass *render_pass_type) {
    render_pass_types.push_back(render_pass_type);
  }
};

#define declare_render_pass(type)                                                                                      \
  virtual const char *get_filename() override;                                                                         \
  static const char *get_filename_static();                                                                            \
  static RenderPassImplBase *get_impl();
#define implement_render_pass(type, filename)                                                                          \
  const char *type::get_filename() {                                                                                   \
    return filename;                                                                                                   \
  }                                                                                                                    \
  const char *type::get_filename_static() {                                                                            \
    return filename;                                                                                                   \
  }                                                                                                                    \
  type g_type##type;                                                                                                   \
  RenderPassImpl<type> g_renderpass##type(&g_type##type);                                                              \
  RenderPassImplBase *type::get_impl() {                                                                               \
    return &g_renderpass##type;                                                                                        \
  }

struct ShadowMapRenderPass : public RenderPass {
  declare_render_pass(ShadowMapRenderPass);
  virtual void get_compile_jobs(Vector<shader::compiler::Input> &inputs) override;
  HashMap<u32, ShaderPair> compiled_shaders;
};

struct GBufferRenderPass : public RenderPass {
  declare_render_pass(GBufferRenderPass);
  virtual void get_compile_jobs(Vector<shader::compiler::Input> &inputs) override {
  }
};

// These generate shaders only for gbuffer
struct MaterialShader : public asset::Asset {
  declare_asset_type(MaterialShader);
  virtual void serialize(Archive &archive) override {
    serialize(archive);
    archive << compiled_shaders;
  }
  static void add(const String &filename);
  HashMap<u32, RenderPass::ShaderPair> compiled_shaders;
  bool ready;
};

struct MaterialShaderFactory : public asset::Factory {
  virtual const char *get_filename_extensions() {
    return "shader";
  }
  virtual asset::AssetRef create(const String &filename) {
    return asset::AssetRef(new MaterialShader);
  }
};
