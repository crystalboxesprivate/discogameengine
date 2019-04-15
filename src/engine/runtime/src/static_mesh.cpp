#include <runtime/static_mesh.h>
#include <graphicsinterface/graphicsinterface.h>
#include <engine>
#include <runtime/static_mesh_resource.h>

namespace runtime {

StaticMeshResource::StaticMeshResource() {
  namespace gi = graphicsinterface;
  vertex_stream.count = 0;
  vertex_stream.add({nullptr, gi::SemanticName::Position, gi::PixelFormat::R32G32B32F, 0u, 0u});
  vertex_stream.add({nullptr, gi::SemanticName::TexCoord, gi::PixelFormat::R32G32F, 0u, 0u});
  vertex_stream.add({nullptr, gi::SemanticName::Normal, gi::PixelFormat::R32G32B32F, 0u, 0u});
  vertex_stream.add({nullptr, gi::SemanticName::Color, gi::PixelFormat::R32G32B32F, 0u, 0u});
}

using namespace graphicsinterface;
StaticMeshResource &StaticMesh::get_render_resource() {
  // if it's not loaded then return default staticmeshresource
  if (!render_data) {
    render_data = SharedPtr<StaticMeshResource>(new StaticMeshResource);
  }

  if (!render_data->lod_resources.size()) {
    // init loading stuff
    if (!is_loading) {
      asset::load_to_ram(*this);
    }

    if (!is_loaded_to_ram) {
      return *asset::get_default<StaticMesh>(asset::Type::StaticMesh).render_data.get();
    }

    usize offset = 0;
    for (auto &lod_item_info : lod_info) {
      StaticMeshRenderData lod_resource;
      {
        lod_resource.indices = graphicsinterface::create_index_buffer(lod_item_info.index_count);
        usize index_byte_size = lod_item_info.index_count * sizeof(i32);
        graphicsinterface::set_index_buffer_data(bulk_data.get_data() + offset, index_byte_size, lod_resource.indices);
        offset = offset + index_byte_size;
      }

      {
        usize stride = sizeof(float) * 3;
        usize vert_count = lod_item_info.vertex_count;
        usize vertex_byte_size = vert_count * stride;
        lod_resource.positions = graphicsinterface::create_vertex_buffer(vert_count, stride, PixelFormat::R32G32B32F);
        graphicsinterface::set_vertex_buffer_data(bulk_data.get_data() + offset, vertex_byte_size,
                                                  lod_resource.positions);
        offset = offset + vertex_byte_size;
      }

      {
        usize stride = sizeof(float) * 2;
        usize vert_count = lod_item_info.vertex_count;
        usize vertex_byte_size = vert_count * stride;
        lod_resource.texcoords = graphicsinterface::create_vertex_buffer(vert_count, stride, PixelFormat::R32G32F);
        graphicsinterface::set_vertex_buffer_data(bulk_data.get_data() + offset, vertex_byte_size,
                                                  lod_resource.texcoords);
        offset = offset + vertex_byte_size;
      }

      {
        usize stride = sizeof(float) * 3;
        usize vert_count = lod_item_info.vertex_count;
        usize vertex_byte_size = vert_count * stride;
        lod_resource.normals = graphicsinterface::create_vertex_buffer(vert_count, stride, PixelFormat::R32G32B32F);
        graphicsinterface::set_vertex_buffer_data(bulk_data.get_data() + offset, vertex_byte_size,
                                                  lod_resource.normals);
        offset = offset + vertex_byte_size;
      }

      if (lod_item_info.has_colors) {
        usize stride = sizeof(float) * 3;
        usize vert_count = lod_item_info.vertex_count;
        usize vertex_byte_size = vert_count * stride;
        lod_resource.colors = graphicsinterface::create_vertex_buffer(vert_count, stride, PixelFormat::R32G32B32F);
        graphicsinterface::set_vertex_buffer_data(bulk_data.get_data() + offset, vertex_byte_size, lod_resource.colors);
        offset = offset + vertex_byte_size;
      }

      render_data->vertex_stream.attributes[0].buffer = lod_resource.positions.get();
      render_data->vertex_stream.attributes[1].buffer = lod_resource.texcoords.get();
      render_data->vertex_stream.attributes[2].buffer = lod_resource.normals.get();
      render_data->vertex_stream.attributes[3].buffer = lod_resource.colors ? lod_resource.colors.get() : nullptr;

      render_data->lod_resources.push_back(lod_resource);
    }
    free();
  }

  return *(render_data.get());
}

void StaticMesh::serialize(Archive &archive) {
  archive << lod_info;
  archive << bulk_data;
}

implement_asset_type(StaticMesh);
} // namespace runtime
