#pragma once

#include <engine>
#include <graphicsinterface/shader.h>

namespace shader {
namespace compiler {
struct Error {
  String shader_name;
  String shader_error;
};

struct ReflectionData {
  struct UniformBuffer {
    struct Member {
      String name;
      usize size;
      usize stride;
      usize offset;
      String type;
    };
    String name;
    usize size;
    HashMap<String, Member> members;
  };
  Vector<UniformBuffer> uniform_buffers;
};

struct Environment {
  Map<String, String> definition_map;
  Map<String, String> virtual_path_to_contents;
};

struct Input {
  String virtual_source_path;
  String entry_point = "main";
  graphicsinterface::ShaderStage shader_stage = graphicsinterface::ShaderStage::NumStages;
  String file_prefix = "";

  Environment environemnt;
  bool skip_preprocessor = false;
};

struct Output {
  ReflectionData reflection_data;
  Vector<Error> errors;
};

struct Definitions {
  Map<String, String> definition_map;
};

} // namespace compiler
} // namespace shader