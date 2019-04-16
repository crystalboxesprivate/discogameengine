#include <ShaderConductor/ShaderConductor.hpp>
#include <spirv_glsl.hpp>
#include <cross/cross.h>
#include <preprocessor/preprocessor.h>

#include <config/shader_target.h>

namespace cross {
extern String g_root_dir;
typedef char *Str;

namespace helpers {
bool starts_with(const String &with_string, const String &in_string) {
  usize iteration_count = with_string.size() > in_string.size() ? in_string.size() : with_string.size();
  for (int x = 0; x < iteration_count; x++) {
    if (in_string[x] != with_string[x])
      return false;
  }
  return true;
}

} // namespace helpers
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

void run_shader_conductor(compiler::Input &input, compiler::Output &output, String &out_data) {
  const String &pre_processed_source = input.source;
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
#if 0
    String compiled = compiler.compile();

    out_data.resize(compiled.size());
    memcpy(out_data.data(), compiled.data(), compiled.size());
#else
    out_data = compiler.compile();
#endif

    compiler::ReflectionData reflection_data;
    spirv_cross_helpers::load_reflection_data(output.reflection_data, compiler);

    if (config::shader_language == shader::ShadingLanguage::Hlsl) {
      // generate the hlsl code
#if 0
      target_description = {ShaderConductor::ShadingLanguage::Dxil, ""};

      ShaderConductor::Compiler::ResultDesc hlsl_res =
          ShaderConductor::Compiler::Compile(source_description, compiler_options, target_description);
      out_data.resize(hlsl_res.target->Size());
      memcpy(out_data.data(), hlsl_res.target->Data(), hlsl_res.target->Size());

#else
      out_data = pre_processed_source;
      // out_data.resize(pre_processed_source.size());
      // memcpy(out_data.data(), pre_processed_source.data(), pre_processed_source.size());
#endif
    }
  }
}

void cross_compile(Vector<compiler::Input> &inputs, Vector<compiler::Output> &compiler_outputs,
                   Vector<String> &out_sources) {
  for (compiler::Input &input : inputs) {
    String cross_compiled_result;
    compiler::Output compiler_output;
    if (input.source.size() == 0) {
      assert(input.virtual_source_path.size());
      preprocessor::run(input.source, compiler_output, input, {});
    }

    run_shader_conductor(input, compiler_output, cross_compiled_result);
    out_sources.push_back(cross_compiled_result);
    compiler_outputs.push_back(compiler_output);
  }
}
} // namespace cross
