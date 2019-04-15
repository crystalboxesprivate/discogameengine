#pragma once

#include <engine>
#include <graphicsinterface/graphicsinterface.h>
#include <shader/shader.h>

namespace cross {
using Stage = graphicsinterface::ShaderStage;
namespace compiler = shader::compiler;
} // namespace cross

namespace cross {
void set_shader_root_dir(const String &dir);

void cross_compile(Vector<compiler::Input> &inputs, Vector<compiler::Output> &compiler_outputs,
                   Vector<String> &out_sources);
} // namespace cross
