add_library(lib_cppgrep STATIC cppgrep/cppgrep.cpp)
target_include_directories(lib_cppgrep PRIVATE
    ${LLVM_INCLUDE_DIR}
    ${PROJECT_SOURCE_DIR}/ext/cxxopts/include
    ${PROJECT_SOURCE_DIR}/ext/termcolor/include)
target_link_libraries(lib_cppgrep PUBLIC ${LLVM_LIBRARY})

add_executable(cppgrep cppgrep/main.cpp)
target_link_libraries(cppgrep PRIVATE project_warnings lib_cppgrep)
