add_library(freetype STATIC lib/freetype.lib)
set_target_properties(freetype PROPERTIES LINKER_LANGUAGE C++)
target_include_directories(freetype PUBLIC include)
