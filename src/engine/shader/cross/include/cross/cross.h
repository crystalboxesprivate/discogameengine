#pragma once

#include <engine>

namespace cross {
enum class Stage : u32 {
  Vertex,
  Pixel,
  Geometry,
  Hull,
  Domain,
  Compute,

  NumStages,
};
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
  Stage shader_stage = Stage::NumStages;
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
} // namespace cross

namespace cross {
void set_shader_root_dir(const String &dir);
namespace preprocessor {
bool run(String &preprocessed_output, compiler::Output &shader_output, const compiler::Input &shader_input,
         const compiler::Definitions &additional_defines);
}
void cross_compile(Vector<compiler::Input> &inputs, Vector<compiler::Output> &compiler_outputs,
                   Vector<String> &out_sources);
} // namespace cross
