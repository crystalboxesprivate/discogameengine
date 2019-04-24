#include <runtime/static_mesh.h>
#include <graphicsinterface/graphicsinterface.h>
#include <engine>
#include <runtime/static_mesh_resource.h>

namespace gi = graphicsinterface;
using gi::PixelFormat;

namespace runtime {
StaticMeshResource::StaticMeshResource() {
  vertex_stream.count = 0;
  vertex_stream.add({0, gi::SemanticName::Position, gi::PixelFormat::R32G32B32F, 0u, 0u});
  vertex_stream.add({1, gi::SemanticName::TexCoord, gi::PixelFormat::R32G32F, 0u, 0u});
  vertex_stream.add({2, gi::SemanticName::Normal, gi::PixelFormat::R32G32B32F, 0u, 0u});
  vertex_stream.add({3, gi::SemanticName::Tangent, gi::PixelFormat::R32G32B32F, 0u, 0u});
  vertex_stream.add({4, gi::SemanticName::Color, gi::PixelFormat::R32G32B32F, 0u, 0u});
}

StaticMeshResource *StaticMesh::get_render_resource() {
  // if it's not loaded then return default staticmeshresource
  if (!render_data) {
    render_data = SharedPtr<StaticMeshResource>(new StaticMeshResource);
  }

  if (!render_data->lod_resources.size()) {
    // init loading stuff
    if (!is_loading) {
      asset::load_to_ram(*this, false, false);
    }

    if (!is_loaded_to_ram) {
      auto default_asset = asset::get_default<StaticMesh>(asset::Type::StaticMesh);
      if (default_asset)
        return default_asset->render_data.get();
      return nullptr;
    }

    usize offset = 0;
    for (auto &lod_item_info : lod_info) {
      StaticMeshRenderData lod_resource;
      {
        lod_resource.indices = gi::create_index_buffer(lod_item_info.index_count);
        usize index_byte_size = lod_item_info.index_count * sizeof(i32);
        gi::set_index_buffer_data(bulk_data.get_data() + offset, index_byte_size, lod_resource.indices);
        offset = offset + index_byte_size;
      }

      {
        usize stride = sizeof(float) * 3;
        usize vert_count = lod_item_info.vertex_count;
        usize vertex_byte_size = vert_count * stride;
        lod_resource.positions = gi::create_vertex_buffer(vert_count, stride, PixelFormat::R32G32B32F);
        gi::set_vertex_buffer_data(bulk_data.get_data() + offset, vertex_byte_size, lod_resource.positions);
        offset = offset + vertex_byte_size;
      }

      {
        usize stride = sizeof(float) * 2;
        usize vert_count = lod_item_info.vertex_count;
        usize vertex_byte_size = vert_count * stride;
        lod_resource.texcoords = gi::create_vertex_buffer(vert_count, stride, PixelFormat::R32G32F);
        gi::set_vertex_buffer_data(bulk_data.get_data() + offset, vertex_byte_size, lod_resource.texcoords);
        offset = offset + vertex_byte_size;
      }

      {
        usize stride = sizeof(float) * 3;
        usize vert_count = lod_item_info.vertex_count;
        usize vertex_byte_size = vert_count * stride;
        lod_resource.normals = gi::create_vertex_buffer(vert_count, stride, PixelFormat::R32G32B32F);
        gi::set_vertex_buffer_data(bulk_data.get_data() + offset, vertex_byte_size, lod_resource.normals);
        offset = offset + vertex_byte_size;
      }

      {
        usize stride = sizeof(float) * 3;
        usize vert_count = lod_item_info.vertex_count;
        usize vertex_byte_size = vert_count * stride;
        lod_resource.tangents = gi::create_vertex_buffer(vert_count, stride, PixelFormat::R32G32B32F);
        gi::set_vertex_buffer_data(bulk_data.get_data() + offset, vertex_byte_size, lod_resource.tangents);
        offset = offset + vertex_byte_size;
      }

      if (lod_item_info.has_colors) {
        usize stride = sizeof(float) * 3;
        usize vert_count = lod_item_info.vertex_count;
        usize vertex_byte_size = vert_count * stride;
        lod_resource.colors = gi::create_vertex_buffer(vert_count, stride, PixelFormat::R32G32B32F);
        gi::set_vertex_buffer_data(bulk_data.get_data() + offset, vertex_byte_size, lod_resource.colors);
        offset = offset + vertex_byte_size;
      }

      render_data->vertex_stream.add_buffer(lod_resource.positions.get());
      render_data->vertex_stream.add_buffer(lod_resource.texcoords.get());
      render_data->vertex_stream.add_buffer(lod_resource.normals.get());
      render_data->vertex_stream.add_buffer(lod_resource.tangents.get());
      if (lod_resource.colors)
        render_data->vertex_stream.add_buffer(lod_resource.colors.get());
      render_data->lod_resources.push_back(lod_resource);
    }
    free();
  }

  return render_data.get();
}

void StaticMesh::serialize(Archive &archive) {
  Asset::serialize(archive);
  archive << lod_info;
  archive << bulk_data;
}

implement_asset_type(StaticMesh);
} // namespace runtime
