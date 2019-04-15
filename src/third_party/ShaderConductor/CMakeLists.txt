# add_library(
#   ShaderConductor
#   lib/ShaderConductor.lib
# )
# SET_TARGET_PROPERTIES(ShaderConductor PROPERTIES LINKER_LANGUAGE C++)
# target_include_directories(ShaderConductor PUBLIC include)
add_pre_built_static_library(ShaderConductor lib/ShaderConductor.lib)
