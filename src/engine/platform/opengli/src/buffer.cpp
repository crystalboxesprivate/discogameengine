#include <opengli/opengli.h>
#include <glad/glad.h>

namespace graphicsinterface {
  IndexBufferRef create_index_buffer(usize count, void *initial_data) {
    GlIndexBuffer &buffer = *new GlIndexBuffer();
    buffer.count = count;
    glGenBuffers(1, &buffer.vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer.vbo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, buffer.get_num_bytes(), initial_data, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    return IndexBufferRef(&buffer);
  }

  VertexBufferRef create_vertex_buffer(usize count, usize element_size, PixelFormat pixel_format, void *initial_data) {
    GlVertexBuffer &buffer = *new GlVertexBuffer();
    buffer.count = count;
    buffer.stride = element_size;
    glGenBuffers(1, &buffer.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, buffer.vbo);
    glBufferData(GL_ARRAY_BUFFER, element_size * count, initial_data, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    return VertexBufferRef(&buffer);
  }
  
  void set_index_buffer_data(void* data, usize byte_count, IndexBufferRef buffer) {
    assert(data);
    auto& index_buffer = *(GlIndexBuffer*) buffer.get();
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer.vbo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, byte_count, data, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  }

  void set_vertex_buffer_data(void* data, usize byte_count, VertexBufferRef buffer) {
    assert(data);
    auto& vertex_buffer = *(GlVertexBuffer*) buffer.get();
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer.vbo);
    glBufferData(GL_ARRAY_BUFFER, byte_count, data, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
  }
}
