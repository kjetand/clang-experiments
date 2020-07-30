add_library(lib_cppgrep STATIC cppgrep/cppgrep.cpp)
target_link_libraries(lib_cppgrep PUBLIC project_warnings ${LIBCLANG_LIBRARY})

add_executable(cppgrep cppgrep/main.cpp)
target_link_libraries(cppgrep PRIVATE project_warnings lib_cppgrep)
