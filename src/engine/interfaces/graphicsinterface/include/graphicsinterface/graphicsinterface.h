#pragma once
#include <graphicsinterface/pixel_format.h>
#include <graphicsinterface/buffer.h>
#include <graphicsinterface/shader.h>
#include <engine>

const u32 GI_CLEAR_DEPTH = 1;
const u32 GI_CLEAR_STENCIL = 2;

const u32 GI_PT_TRIANGLE_STRIP = 4;

#define graphics_debug_state(msg)

namespace graphicsinterface {
static const i32 VERTEX_ATTRIBUTE_COUNT_MAX = 10;
enum class PrimitiveTopology : u8 { PointList, LineList, LineStrip, TriangleList, TriangleStrip };

struct DebugState {
  DebugState(const char* title);
  ~DebugState();
  static void set_marker(const char* text);
};

struct PipelineState {
  struct BoundShaders {
    ShaderRef vertex;
    ShaderRef pixel;
  };
  PrimitiveTopology primitive_type;
  BoundShaders bound_shaders;
};
enum class SemanticName : u8 { Position, TexCoord, Normal, Color };
struct VertexAttribute {
  VertexBuffer *buffer;
  SemanticName semantic_name;
  PixelFormat pixel_format;
  u32 aligned_byte_offset;
  u32 semantic_index;
};

struct VertexStream {
  void add(const VertexAttribute &attribute) {
    attributes[count++] = attribute;
  }
  VertexAttribute attributes[VERTEX_ATTRIBUTE_COUNT_MAX];
  usize count = 0;
};

void set_pipeline_state(PipelineState &pipeline_state);

void bind_uniform_buffer(u32 slot, ShaderStage stage, UniformBufferRef uniform_buffer);
void set_vertex_stream(VertexStream &stream, ShaderRef VertexShader);
void draw_indexed(IndexBufferRef index_buffer, PrimitiveTopology primitive_type, u32 index_count, u32 start_index, u32 vertex_start_index);

struct RenderTargetView {
  virtual void *get_native_ptr() = 0;
};
struct DepthStencilView {
  virtual void *get_native_ptr() = 0;
};

typedef SharedPtr<RenderTargetView> RenderTargetViewRef;
typedef SharedPtr<DepthStencilView> DepthStencilViewRef;

RenderTargetView &get_main_render_target_view();
DepthStencilView &get_main_depth_stencil_view();

RenderTargetViewRef create_render_target_view(usize width, usize height, PixelFormat pixelformat);
DepthStencilViewRef create_depth_stencil_view(usize width, usize height, PixelFormat pixelformat);

bool delete_context();
bool create_context(void *window_ptr, void *data);

void clear_render_target_view(RenderTargetView &view, const glm::vec4 &clear_color);
void clear_depth_stencil_view(DepthStencilView &view, u32 clear_flags, float depth = 1.f, float stencil = 0.f);
void set_viewport(u32 x, u32 y, usize width, usize height);

IndexBufferRef create_index_buffer(usize count, void *initial_data = nullptr);
VertexBufferRef create_vertex_buffer(usize count, usize element_size, PixelFormat pixel_format,
                                     void *initial_data = nullptr);
void set_index_buffer_data(void *data, usize byte_count, IndexBufferRef buffer);
void set_vertex_buffer_data(void *data, usize byte_count, VertexBufferRef buffer);
void set_uniform_buffer_data(void *data, usize byte_count, UniformBufferRef buffer);

ShaderRef create_shader(ShaderDescription &shader);

void *get_native_device();
void *get_native_context();

void present();
} // namespace graphicsinterface
