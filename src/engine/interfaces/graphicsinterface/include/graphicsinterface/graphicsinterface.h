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
  DebugState(const char *title);
  ~DebugState();
  static void set_marker(const char *text);
};

struct PipelineState {
  struct BoundShaders {
    ShaderRef vertex;
    ShaderRef pixel;
  };
  PrimitiveTopology primitive_type;
  BoundShaders bound_shaders;
};
enum class SemanticName : u8 { Position, TexCoord, Normal, Tangent, Color };
struct VertexAttribute {
  u8 buffer_slot;
  SemanticName semantic_name;
  PixelFormat pixel_format;
  u32 aligned_byte_offset;
  u32 semantic_index;
};

struct VertexStream {
  void add(const VertexAttribute &attribute) {
    attributes[count++] = attribute;
  }
  void add_buffer(VertexBuffer *buffer) {
    assert(buffer);
    buffers[buffer_count++] = buffer;
  }
  VertexAttribute attributes[VERTEX_ATTRIBUTE_COUNT_MAX];
  VertexBuffer *buffers[VERTEX_ATTRIBUTE_COUNT_MAX];
  usize count = 0;
  usize buffer_count = 0;
};

void set_pipeline_state(PipelineState &pipeline_state);

void bind_uniform_buffer(u32 slot, ShaderStage stage, UniformBufferRef uniform_buffer);
void set_vertex_stream(VertexStream &stream, ShaderRef VertexShader);
void draw_indexed(IndexBufferRef index_buffer, PrimitiveTopology primitive_type, u32 index_count, u32 start_index,
                  u32 vertex_start_index);
void draw(u32 vertex_count, u32 vertex_start_location);

struct Texture2D {
  virtual void *get_native_ptr() = 0;
  virtual u16 get_width() = 0;
  virtual u16 get_height() = 0;
  virtual PixelFormat get_pixel_format() = 0;
};

struct ShaderResourceView {
  virtual void *get_native_ptr() = 0;
};

struct RenderTargetView {
  virtual void *get_native_ptr() = 0;
};
struct DepthStencilView {
  virtual void *get_native_ptr() = 0;
};

struct SamplerState {
  virtual void *get_native_ptr() = 0;
};

typedef SharedPtr<RenderTargetView> RenderTargetViewRef;
typedef SharedPtr<DepthStencilView> DepthStencilViewRef;
typedef SharedPtr<ShaderResourceView> ShaderResourceViewRef;
typedef SharedPtr<Texture2D> Texture2DRef;
typedef SharedPtr<SamplerState> SamplerStateRef;

RenderTargetViewRef get_main_render_target_view();
DepthStencilViewRef get_main_depth_stencil_view();

SamplerStateRef create_sampler_state();
Texture2DRef create_texture2d(usize width, usize height, PixelFormat pixelformat);
ShaderResourceViewRef create_shader_resource_view(Texture2DRef texture2d);
RenderTargetViewRef create_render_target_view(Texture2DRef texture2d);

void set_render_targets(i32 num_render_targets, RenderTargetViewRef *render_targets,
                        DepthStencilViewRef depth_stencil_view);

// Todo change / fix this
DepthStencilViewRef create_depth_stencil_view(usize width, usize height, PixelFormat pixelformat);

bool delete_context();
bool create_context(void *window_ptr, void *data);

struct ShaderParameter {
  i32 base_index = 0;
};

void set_shader_resource_view(const ShaderParameter &shader_parameter, ShaderResourceViewRef srv, ShaderRef shader);
void set_sampler_state(const ShaderParameter &shader_parameter, SamplerStateRef sampler_state, ShaderRef shader);

void set_z_buffer(bool enabled);

struct BlendState {};
typedef SharedPtr<BlendState> BlendStateRef;

//
// Blending constants
extern const int ZERO;
extern const int ONE;
extern const int SRC_COLOR;
extern const int ONE_MINUS_SRC_COLOR;
extern const int DST_COLOR;
extern const int ONE_MINUS_DST_COLOR;
extern const int SRC_ALPHA;
extern const int ONE_MINUS_SRC_ALPHA;
extern const int DST_ALPHA;
extern const int ONE_MINUS_DST_ALPHA;
extern const int SRC_ALPHA_SATURATE;

extern const int BM_ADD;
extern const int BM_SUBTRACT;
extern const int BM_REVERSE_SUBTRACT;
extern const int BM_MIN;
extern const int BM_MAX;

// Mask constants
#define RED 0x1
#define GREEN 0x2
#define BLUE 0x4
#define ALPHA 0x8

#define ALL (RED | GREEN | BLUE | ALPHA)

BlendStateRef addBlendState(const int srcFactorRGB, const int destFactorRGB, const int srcFactorAlpha,
                            const int destFactorAlpha, const int blendModeRGB, const int blendModeAlpha,
                            const int mask = ALL, const bool alphaToCoverage = false);

BlendStateRef addBlendState(const int srcFactor, const int destFactor, const int blendMode = BM_ADD,
                            const int mask = ALL, const bool alphaToCoverage = false);
void set_blend_state(BlendStateRef blend_state);

void clear_render_target_view(RenderTargetViewRef view, const glm::vec4 &clear_color);
void clear_depth_stencil_view(DepthStencilViewRef view, u32 clear_flags, float depth = 1.f, float stencil = 0.f);
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
