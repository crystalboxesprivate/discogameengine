#include <runtime/skinned_mesh.h>
#include <utils/string.h>

using utils::string::hash_code;
using namespace glm;

using namespace runtime;

namespace gi = graphicsinterface;
using gi::PixelFormat;

using runtime::animation::calculate_glm_interpolated_position;
using runtime::animation::calculate_glm_interpolated_rotation;
using runtime::animation::calculate_glm_interpolated_scaling;
using runtime::animation::find_position;
using runtime::animation::find_rotation;
using runtime::animation::find_scaling;

SkinnedMeshResource::SkinnedMeshResource() {
  namespace gi = graphicsinterface;
  vertex_stream.count = 0;
  vertex_stream.add({0, gi::SemanticName::Position, PixelFormat::FloatRGBA, 0u, 0u});
  vertex_stream.add({0, gi::SemanticName::Normal, PixelFormat::FloatRGBA, 16, 0u});
  vertex_stream.add({0, gi::SemanticName::TexCoord, PixelFormat::FloatRGBA, 32, 0u});

  vertex_stream.add({0, gi::SemanticName::Tangent, PixelFormat::FloatRGBA, 48, 0u});
  vertex_stream.add({0, gi::SemanticName::Binormal, PixelFormat::FloatRGBA, 64, 0u});

  vertex_stream.add({0, gi::SemanticName::TexCoord, PixelFormat::FloatRGBA, 80, 1});
  vertex_stream.add({0, gi::SemanticName::TexCoord, PixelFormat::FloatRGBA, 96, 2});
}

void SkinnedMesh::serialize(Archive &archive) {
  Asset::serialize(archive);
  archive << indices;
  archive << vertices;
  archive << bounds;

  archive << number_of_bones;
  archive << ticks_per_second;

  archive << animations;
  archive << global_inverse_transformation;

  archive << bone_offsets;


  archive << hierarchy;
}

SkinnedMeshResource *SkinnedMesh::get_render_resource() {
  // if it's not loaded then return default SkinnedMeshresource
  if (!render_data) {
    render_data = SharedPtr<SkinnedMeshResource>(new SkinnedMeshResource);
  }

  if (!render_data->index_buffer) {
    if (!is_loading) {
      asset::load_to_ram(*this, false, false);
    }

    if (!is_loaded_to_ram) {
      auto default_asset = asset::get_default<SkinnedMesh>(asset::Type::SkinnedMesh);
      if (default_asset)
        return default_asset->render_data.get();
      return nullptr;
    }

    render_data->index_buffer = gi::create_index_buffer(indices.size());
    usize index_byte_size = indices.size() * sizeof(i32);
    gi::set_index_buffer_data(indices.data(), index_byte_size, render_data->index_buffer);

    usize stride = sizeof(SkinnedMeshVertex);
    render_data->vertex_buffer = gi::create_vertex_buffer(vertices.size(), stride, PixelFormat::FloatRGBA);
    usize vertex_byte_size = vertices.size() * stride;
    gi::set_vertex_buffer_data(vertices.data(), vertex_byte_size, render_data->vertex_buffer);
    free();

    render_data->vertex_stream.add_buffer(render_data->vertex_buffer.get());
  }

  return render_data.get();
}
