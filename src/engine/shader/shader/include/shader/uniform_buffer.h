#pragma once

#include <graphicsinterface/buffer.h>
#include <shader/compiler.h>

namespace shader {
struct UniformBufferDescription {
  // unsigned int gl_id;
  // unsigned int gl_binding_point;
  usize unique_id = 0;
  graphicsinterface::UniformBufferRef resource;
  void update_resource();
  graphicsinterface::UniformBufferRef get_resource() {
    assert(resource);
    return resource;
  }
  struct MemberData {
    usize size;
    usize element_stride;
    usize offset;
    u8 *data_ptr;
    // for arrays and matrices
    bool containts_children() {
      return size != element_stride;
    }

    inline friend Archive &operator<<(Archive &archive, MemberData &desc) {
      archive << desc.size;
      archive << desc.element_stride;
      archive << desc.offset;
      return archive;
    }
  };

  inline friend Archive &operator<<(Archive &archive, UniformBufferDescription &desc) {
    archive << desc.unique_id;
    archive << desc.size;
    archive << desc.name;
    archive << desc.members;
    archive << desc.name_to_parameter_id;

    if (archive.is_loading()) {
      desc.allocate_storage();
    }
    return archive;
  }

  void allocate_storage() {
    assert(!storage_ptr);
    storage_ptr = (u8 *)malloc(size);
    for (auto &member : members) {
      member.data_ptr = storage_ptr + member.offset;
    }
  }

  UniformBufferDescription()
      : storage_ptr(0)
      , size(0)
      , resource(nullptr) {
  }

  ~UniformBufferDescription() {
    if (storage_ptr) {
      free(storage_ptr);
      storage_ptr = nullptr;
    }
  }

  const String get_uniform_name() {
    return "type_" + name;
  }

  void set_raw_parameter(const String &name, void *data, usize size = 0) {
    MemberData *member = get_member(name);
    if (!member) {
      printf("didn't find %s", name.c_str());
      return;
    }
    MemberData &member_data = *member;
    {
      size = size == 0 ? member_data.size : size;
      size = size > member_data.size ? member_data.size : size;
    }
    memcpy(member_data.data_ptr, data, size);
  }

  usize size;
  String name;
  u8 *storage_ptr;

  HashMap<String, i32> name_to_parameter_id;
  Vector<MemberData> members;

  MemberData *get_member(const String &name) {
    typedef HashMap<String, i32>::iterator IteratorType;
    IteratorType result = name_to_parameter_id.find(name);
    if (result == name_to_parameter_id.end())
      return nullptr;
    i32 res = result->second;
    return &members[res];
  }
};
typedef UniquePtr<UniformBufferDescription> UniformBufferDescPtr;
bool initialize_uniform_buffer_members(UniformBufferDescription &buffer,
                                       compiler::ReflectionData::UniformBuffer &reflection_data);
} // namespace shader
