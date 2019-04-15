#include <ShaderConductor/ShaderConductor.hpp>
#include <spirv_glsl.hpp>
#include <mcpp/mcpp.h>
#include <cross/cross.h>
#include <utility>
#include <fstream>

namespace cross {
extern String g_root_dir;
typedef char* Str;

namespace helpers {
bool starts_with(const String &with_string, const String &in_string) {
  usize iteration_count = with_string.size() > in_string.size() ? in_string.size() : with_string.size();
  for (int x = 0; x < iteration_count; x++) {
    if (in_string[x] != with_string[x])
      return false;
  }
  return true;
}
void memswap(void *ptr1, void *ptr2, size_t size) {
  std::swap_ranges((u8 *)ptr1, (u8 *)ptr1 + size, (u8 *)ptr2);
}
// https://stackoverflow.com/questions/2342162/stdstring-formatting-like-sprintf/26221725#26221725
#pragma warning(disable : 4996)
template <typename... Args>
String sprintf(const String &format, Args... args) {
  size_t size = snprintf(nullptr, 0, format.c_str(), args...) + 1;
  std::unique_ptr<char[]> buf(new char[size]);
  snprintf(buf.get(), size, format.c_str(), args...);
  return String(buf.get(), buf.get() + size - 1);
}

bool load_file_to_string(const String &filename, String &contents) {
  std::ifstream input_stream(filename);
  if (input_stream) {
    input_stream.seekg(0, std::ios::end);
    size_t char_count = input_stream.tellg();
    input_stream.seekg(0);
    contents = String(char_count + 1, '\0');
    input_stream.read(&contents[0], char_count);
    return true;
  }
  return false;
}

bool virtual_path_to_actual_path(const char *virtual_path, String &out_path) {
  out_path = /* g_root_dir + "/" + */ virtual_path;
  return true;
}

bool load_shader_source(const char *virtual_path, String &contents, Vector<compiler::Error> *out_compiler_error) {
  String filename;
  if (!virtual_path_to_actual_path(virtual_path, filename)) {
    if (out_compiler_error)
      out_compiler_error->push_back({String("Error: Couldn't locate the virtual path for: ") + virtual_path});
    return false;
  }

  if (!load_file_to_string(filename, contents)) {
    if (out_compiler_error)
      out_compiler_error->push_back(
          {String("Error: Couldn't read file: ") + filename + " for virtual shader path " + virtual_path});
    return false;
  }
  return true;
}

Vector<char> string_to_array(String &in_string) {
  Vector<char> out_vector(in_string.size() + 1);
  memcpy(out_vector.data(), in_string.data(), in_string.size());
  out_vector[in_string.size()] = '\0';
  return out_vector;
}
} // namespace helpers
} // namespace cross

namespace cross {
namespace preprocessor {
static void add_mcpp_definitions(String &OutOptions, const Map<String, String> &Definitions) {
  for (Map<String, String>::const_iterator it = Definitions.begin(); it != Definitions.end(); ++it)
    OutOptions += helpers::sprintf(" \"-D%s=%s\"", it->first.c_str(), it->second.c_str());
}

static bool read_mcpp_errors(Vector<compiler::Error> &out_errors, const String &mcpp_errors) {
  if (mcpp_errors.size() > 0) {
    out_errors.push_back({"Preprocessor error: " + mcpp_errors});
    return true;
  }
  return false;
}

struct McppLoader {
  explicit McppLoader(const compiler::Input &in_shader_input, compiler::Output &in_shader_output)
      : shader_input(in_shader_input)
      , shader_output(in_shader_output) {
    String input_shader_source;
    if (helpers::load_shader_source(in_shader_input.virtual_source_path.c_str(), input_shader_source, nullptr)) {
      // input_shader_source = "#line 1 \n" + input_shader_source;
      input_shader_source =
          helpers::sprintf("%s\n#line 1\n%s", shader_input.file_prefix.c_str(), input_shader_source.c_str());
      cached_contents[in_shader_input.virtual_source_path] = helpers::string_to_array(input_shader_source);
    }
  }

  file_loader get_mcpp_callback() {
    file_loader Loader;
    Loader.get_file_contents = get_file_contents;
    Loader.user_data = (void *)this;
    return Loader;
  }

private:
  typedef Vector<char> ShaderContents;
  static int get_file_contents(void *file_loader_ptr, const char *in_virtual_path, const char **out_contents,
                               size_t *out_contents_size) {
    McppLoader &self = *(McppLoader *)file_loader_ptr;
    String virtual_file_path = String(in_virtual_path);

    // TODO handle relative dirs : ../shader.hlsl
    ShaderContents *cached_shader_contents;
    auto contents_it = self.cached_contents.find(virtual_file_path);
    if (contents_it == self.cached_contents.end())
      cached_shader_contents = nullptr;
    else
      cached_shader_contents = &contents_it->second;

    if (!cached_shader_contents) {
      String file_contents;
      auto virtual_contents_it = self.shader_input.environemnt.virtual_path_to_contents.find(virtual_file_path);
      if (virtual_contents_it != self.shader_input.environemnt.virtual_path_to_contents.end()) {
        file_contents = virtual_contents_it->second;
      }
      helpers::load_shader_source(virtual_file_path.c_str(), file_contents, &self.shader_output.errors);

      if (file_contents.size() > 0) {
        file_contents = helpers::sprintf(("#line 1 \"%s\"\n%s"), virtual_file_path.c_str(), file_contents.c_str());
        self.cached_contents[virtual_file_path] = helpers::string_to_array(file_contents);
        cached_shader_contents = &self.cached_contents.find(virtual_file_path)->second;
      }
    }

    if (out_contents)
      *out_contents = cached_shader_contents ? cached_shader_contents->data() : nullptr;
    if (out_contents_size)
      *out_contents_size = cached_shader_contents ? cached_shader_contents->size() : 0;

    return cached_shader_contents != nullptr;
  }
  const compiler::Input &shader_input;
  compiler::Output &shader_output;
  Map<String, ShaderContents> cached_contents;
};

bool run(String &preprocessed_output, compiler::Output &shader_output, const compiler::Input &shader_input,
         const compiler::Definitions &additional_defines) {
  if (shader_input.skip_preprocessor)
    return helpers::load_shader_source(shader_input.virtual_source_path.c_str(), preprocessed_output, nullptr);
  String mcpp_options;
  String mcpp_output, mcpp_errors;
  Str mcpp_out_str = nullptr;
  Str mcpp_error_str = nullptr;

  McppLoader loader(shader_input, shader_output);
  add_mcpp_definitions(mcpp_options, shader_input.environemnt.definition_map);
  add_mcpp_definitions(mcpp_options, additional_defines.definition_map);

  mcpp_options += " -V199901L";
  i32 Result = mcpp_run(mcpp_options.data(), shader_input.virtual_source_path.data(), &mcpp_out_str, &mcpp_error_str,
                        loader.get_mcpp_callback());

  if (!mcpp_out_str)
    return false;

  mcpp_output = mcpp_out_str;
  if (mcpp_error_str) {
    mcpp_errors = mcpp_error_str;
    read_mcpp_errors(shader_output.errors, mcpp_errors);
  }

  helpers::memswap(&preprocessed_output, &mcpp_output, sizeof(String));
  return true;
}
} // namespace preprocessor
} // namespace cross

namespace cross {
String g_root_dir = "";

namespace spirv_cross_helpers {
#pragma region Type conversion
String type_to_glsl(const spirv_cross::SPIRType &type) {
  using namespace spirv_cross;
  using namespace std;
  switch (type.basetype) {
  case SPIRType::Struct:
    return "SPIRType::Struct";
  case SPIRType::Image:
  case SPIRType::SampledImage:
    return "SPIRType::SampledImage";

  case SPIRType::Sampler:
    return /* comparison_ids.count(id) ? "SamplerComparisonState" : */ "SamplerState";

  case SPIRType::Void:
    return "void";
  default:
    break;
  }

  if (type.vecsize == 1 && type.columns == 1) // Scalar builtin
  {
    switch (type.basetype) {
    case SPIRType::Boolean:
      return "bool";
    case SPIRType::Int:
      return "int"; // backend.basic_int_type;
    case SPIRType::UInt:
      return "uint"; // backend.basic_uint_type;
    case SPIRType::AtomicCounter:
      return "atomic_uint";
    case SPIRType::Half:
      return "min16float";
    case SPIRType::Float:
      return "float";
    case SPIRType::Double:
      return "double";
    case SPIRType::Int64:
      return "int64_t";
    case SPIRType::UInt64:
      return "uint64_t";
    default:
      return "???";
    }
  } else if (type.vecsize > 1 && type.columns == 1) // Vector builtin
  {
    switch (type.basetype) {
    case SPIRType::Boolean:
      return join("bool", type.vecsize);
    case SPIRType::Int:
      return join("int", type.vecsize);
    case SPIRType::UInt:
      return join("uint", type.vecsize);
    case SPIRType::Half:
      return join("min16float", type.vecsize);
    case SPIRType::Float:
      return join("float", type.vecsize);
    case SPIRType::Double:
      return join("double", type.vecsize);
    case SPIRType::Int64:
      return join("i64vec", type.vecsize);
    case SPIRType::UInt64:
      return join("u64vec", type.vecsize);
    default:
      return "???";
    }
  } else {
    switch (type.basetype) {
    case SPIRType::Boolean:
      return join("bool", type.columns, "x", type.vecsize);
    case SPIRType::Int:
      return join("int", type.columns, "x", type.vecsize);
    case SPIRType::UInt:
      return join("uint", type.columns, "x", type.vecsize);
    case SPIRType::Half:
      return join("min16float", type.columns, "x", type.vecsize);
    case SPIRType::Float:
      return join("float", type.columns, "x", type.vecsize);
    case SPIRType::Double:
      return join("double", type.columns, "x", type.vecsize);
    // Matrix types not supported for int64/uint64.
    default:
      return "???";
    }
  }
}
#pragma endregion

spv::ExecutionModel get_execution_model_from_shader_stage(Stage shader_stage) {
  ShaderConductor::ShaderStage stage = (ShaderConductor::ShaderStage)shader_stage;
  switch (stage) {
  case ShaderConductor::ShaderStage::VertexShader:
  default:
    return spv::ExecutionModelVertex;

  case ShaderConductor::ShaderStage::HullShader:
    return spv::ExecutionModelTessellationControl;

  case ShaderConductor::ShaderStage::DomainShader:
    return spv::ExecutionModelTessellationEvaluation;

  case ShaderConductor::ShaderStage::GeometryShader:
    return spv::ExecutionModelGeometry;

  case ShaderConductor::ShaderStage::PixelShader:
    return spv::ExecutionModelFragment;

  case ShaderConductor::ShaderStage::ComputeShader:
    return spv::ExecutionModelGLCompute;
  };
}

void configure_glsl_compiler(spirv_cross::CompilerGLSL &compiler,
                             ShaderConductor::Compiler::TargetDesc &target_description) {
  spirv_cross::CompilerGLSL::Options opts = compiler.get_common_options();
  opts.version = 330;
  opts.es = (target_description.language == ShaderConductor::ShadingLanguage::Essl);
  opts.force_temporary = false;
  opts.separate_shader_objects = true;
  opts.flatten_multidimensional_arrays = false;
  opts.enable_420pack_extension = (target_description.language == ShaderConductor::ShadingLanguage::Glsl) &&
                                  ((target_description.version == nullptr) || (opts.version >= 420));

  opts.vulkan_semantics = false;
  opts.vertex.fixup_clipspace = false;
  opts.vertex.flip_vert_y = false;
  opts.vertex.support_nonzero_base_instance = true;
  compiler.set_common_options(opts);

  // build_dummy_sampler
  {
    const uint32_t sampler = compiler.build_dummy_sampler_for_combined_images();
    if (sampler != 0) {
      compiler.set_decoration(sampler, spv::DecorationDescriptorSet, 0);
      compiler.set_decoration(sampler, spv::DecorationBinding, 0);
    }
  }
  // combined_image_samplers
  {
    compiler.build_combined_image_samplers();
    for (auto &remap : compiler.get_combined_image_samplers())
      compiler.set_name(remap.combined_id, compiler.get_name(remap.image_id));
  }
}

void load_reflection_data(compiler::ReflectionData &data, spirv_cross::CompilerGLSL &compiler) {
  typedef std::unordered_set<uint32_t> ActiveVariablesSet;
  ActiveVariablesSet active = compiler.get_active_interface_variables();
  using namespace spirv_cross;
  ShaderResources resources = compiler.get_shader_resources(active);

  for (const Resource &resource : resources.uniform_buffers) {
    compiler::ReflectionData::UniformBuffer refl_buffer;
    {
      const String &name = resource.name;
      const char *prefix = "type_";
      usize prefix_size = strlen(prefix);
      // parse uniform buffer name
      refl_buffer.name =
          helpers::starts_with(prefix, name) ? name.substr(prefix_size, name.size() - prefix_size) : name;
    }

    auto &type = compiler.get_type(resource.base_type_id);
    refl_buffer.size = compiler.get_declared_struct_size(type);

    unsigned member_count = (unsigned)type.member_types.size();
    for (unsigned i = 0; i < member_count; i++) {
      compiler::ReflectionData::UniformBuffer::Member refl_member;
      auto &member_type = compiler.get_type(type.member_types[i]);
      refl_member.type = type_to_glsl(member_type);
      refl_member.stride = refl_member.size = compiler.get_declared_struct_member_size(type, i);
      // Get member offset within this struct.
      refl_member.offset = compiler.type_struct_member_offset(type, i);

      if (!member_type.array.empty()) {
        // Get array stride, e.g. float4 foo[]; Will have array stride of 16 bytes.
        refl_member.stride = compiler.type_struct_member_array_stride(type, i);
      }

      if (member_type.columns > 1) {
        // Get bytes stride between columns (if column major), for float4x4 -> 16 bytes.
        refl_member.stride = compiler.type_struct_member_matrix_stride(type, i);
      }
      refl_member.name = compiler.get_member_name(type.self, i);
      refl_buffer.members[refl_member.name] = refl_member;
    }
    data.uniform_buffers.push_back(refl_buffer);
  }
}
} // namespace spirv_cross_helpers

void set_shader_root_dir(const String &in_root_dir) {
  g_root_dir = in_root_dir;
}

void run_shader_conductor(const String &pre_processed_source, compiler::Input &input, compiler::Output &output,
                          String &out_source) {
  ShaderConductor::Compiler::SourceDesc source_description;
  ShaderConductor::Compiler::TargetDesc target_description;

  source_description = {pre_processed_source.c_str(), input.virtual_source_path.c_str(), input.entry_point.c_str(),
                        (ShaderConductor::ShaderStage)input.shader_stage};
  target_description = {ShaderConductor::ShadingLanguage::SpirV, ""};

  ShaderConductor::Compiler::Options compiler_options;
  compiler_options.packMatricesInRowMajor = false;

  ShaderConductor::Compiler::ResultDesc cc_result =
      ShaderConductor::Compiler::Compile(source_description, compiler_options, target_description);

  if (cc_result.hasError)
    output.errors.push_back(
        {input.virtual_source_path, String("Cross Compiler: ") + (char *)cc_result.errorWarningMsg->Data()});
  else {
    const uint32_t *spirv_ir = reinterpret_cast<const uint32_t *>(cc_result.target->Data());
    const size_t spirv_size = cc_result.target->Size() / sizeof(uint32_t);

    spirv_cross::CompilerGLSL compiler(spirv_ir, spirv_size);
    spv::ExecutionModel model =
        spirv_cross_helpers::get_execution_model_from_shader_stage((Stage)source_description.stage);
    compiler.set_entry_point(source_description.entryPoint, model);
    spirv_cross_helpers::configure_glsl_compiler(compiler, target_description);
    out_source = compiler.compile();

    compiler::ReflectionData reflection_data;
    spirv_cross_helpers::load_reflection_data(output.reflection_data, compiler);
  }
}

void cross_compile(Vector<compiler::Input> &inputs, Vector<compiler::Output> &compiler_outputs,
                   Vector<String> &out_sources) {
  for (compiler::Input &input : inputs) {
    String pre_processed_result, cross_compiled_result;
    compiler::Output compiler_output;

    preprocessor::run(pre_processed_result, compiler_output, input, {});
    run_shader_conductor(pre_processed_result, input, compiler_output, cross_compiled_result);

    out_sources.push_back(cross_compiled_result);
    compiler_outputs.push_back(compiler_output);
  }
}
} // namespace cross
