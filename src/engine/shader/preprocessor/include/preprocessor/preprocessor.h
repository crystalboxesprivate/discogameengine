#pragma once

#include <engine>
#include <shader/compiler.h>

namespace preprocessor {
bool run(String &preprocessed_output, shader::compiler::Output &shader_output, const shader::compiler::Input &shader_input,
         const shader::compiler::Definitions &additional_defines);
}
