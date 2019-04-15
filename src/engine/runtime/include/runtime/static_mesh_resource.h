#include <graphicsinterface/graphicsinterface.h>
#include <runtime/static_mesh.h>

namespace runtime {

// using namespace graphicsinterface;
struct StaticMeshRenderData {
  // buffers
  SharedPtr<graphicsinterface::IndexBuffer> indices;
  SharedPtr<graphicsinterface::VertexBuffer> positions;
  SharedPtr<graphicsinterface::VertexBuffer> texcoords;
  SharedPtr<graphicsinterface::VertexBuffer> normals;
  SharedPtr<graphicsinterface::VertexBuffer> colors;

  void init_resource(StaticMesh &static_mesh);

};

struct StaticMeshResource {
  StaticMeshResource();
  graphicsinterface::VertexStream vertex_stream;
  Vector<StaticMeshRenderData> lod_resources;
};
} // namespace runtime
