# add_library(assimp STATIC lib/assimp-vc140-mt.lib)
# set_target_properties(assimp PROPERTIES LINKER_LANGUAGE C++)
# target_include_directories(assimp PUBLIC include)
add_pre_built_static_library(assimp lib/assimp.lib)
