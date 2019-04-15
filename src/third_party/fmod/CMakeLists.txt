# add_library(fmod STATIC lib/fmod_vc.lib)
# set_target_properties(fmod PROPERTIES LINKER_LANGUAGE C++)
# target_include_directories(fmod PUBLIC include)

add_pre_built_static_library(fmod lib/fmod_vc.lib)
