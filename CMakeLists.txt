cmake_minimum_required(VERSION 3.15)
project(klang CXX)

if (UNIX)
  message(FATAL_ERROR "Unix not yet supported")
endif ()

set(CMAKE_CXX_STANDARD 20)

add_library(project_warnings INTERFACE)

if (MSVC)
  target_compile_options(project_warnings INTERFACE /W4 /WX "/permissive-")

  if (NOT LLVM_ROOT)
    message(FATAL_ERROR "LLVM_ROOT is missing")
  endif ()

  set(LLVM_INCLUDE_DIR ${LLVM_ROOT}/include)
  set(LLVM_LIBRARY ${LLVM_ROOT}/lib/libclang.lib)
endif ()

add_subdirectory(src)
add_subdirectory(test)