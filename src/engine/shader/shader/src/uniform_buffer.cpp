#include <shader/uniform_buffer.h>

namespace shader {
void UniformBufferDescription::update_resource() {
  if (!resource) {
    assert(storage_ptr.get_data());
    resource = graphicsinterface::create_uniform_buffer(size, storage_ptr.get_data());
  }
  graphicsinterface::set_uniform_buffer_data(storage_ptr.get_data(), size, resource);
}

bool initialize_uniform_buffer_members(UniformBufferDescription &buffer,
                                       compiler::ReflectionData::UniformBuffer &reflection_data) {
  if (buffer.size == 0)
    return false;

  buffer.name = reflection_data.name;
  buffer.storage_ptr.allocate(buffer.size);

  typedef compiler::ReflectionData::UniformBuffer::Member Member;
  typedef std::pair<const String, Member> Pair;
  for (Pair &it : reflection_data.members) {
    Member &member = it.second;
    assert(member.size > 0);
    assert(member.stride > 0);
    assert(member.offset < buffer.size);

    buffer.name_to_parameter_id[member.name] = (i32)buffer.members.size();
    buffer.members.push_back({member.size, member.stride, member.offset});
  }
  return true;
}
} // namespace shader
