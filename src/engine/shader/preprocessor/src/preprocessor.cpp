#include <preprocessor/preprocessor.h>
#include <mcpp/mcpp.h>
#include <utils/helpers.h>
#include <utils/fs.h>
#include <utils/string.h>

#include <utility>
#include <fstream>
#include <algorithm>

using namespace shader;

namespace helpers {
void memswap(void *ptr1, void *ptr2, size_t size) {
  std::swap_ranges((u8 *)ptr1, (u8 *)ptr1 + size, (u8 *)ptr2);
}

bool load_shader_source(const char *virtual_path, String &contents, Vector<compiler::Error> *out_compiler_error) {
  String filename;

  if (!utils::fs::load_file_to_string(virtual_path, contents)) {
    if (out_compiler_error)
      out_compiler_error->push_back(
          {String("Error: Couldn't read file: ") + filename + " for virtual shader path " + virtual_path});
    return false;
  }
  return true;
}
} // namespace helpers

namespace preprocessor {
static void add_mcpp_definitions(String &OutOptions, const Map<String, String> &Definitions) {
  for (Map<String, String>::const_iterator it = Definitions.begin(); it != Definitions.end(); ++it)
    OutOptions += utils::string::sprintf(" \"-D%s=%s\"", it->first.c_str(), it->second.c_str());
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
          utils::string::sprintf("%s\n#line 1\n%s", shader_input.file_prefix.c_str(), input_shader_source.c_str());
      cached_contents[in_shader_input.virtual_source_path] = utils::string::to_array(input_shader_source);
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
        file_contents =
            utils::string::sprintf(("#line 1 \"%s\"\n%s"), virtual_file_path.c_str(), file_contents.c_str());
        self.cached_contents[virtual_file_path] = utils::string::to_array(file_contents);
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
  char *mcpp_out_str = nullptr;
  char *mcpp_error_str = nullptr;

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
