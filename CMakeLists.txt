cmake_minimum_required(VERSION 3.15)
project(klang CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(project_warnings INTERFACE)

if (MSVC)
  message(STATUS "Building for Windows")
  target_compile_options(project_warnings INTERFACE /W4 /WX "/permissive-")

  if (NOT LLVM_ROOT)
    message(FATAL_ERROR "LLVM_ROOT is missing")
  endif ()

  set(LIBCLANG_INCLUDE_DIR ${LLVM_ROOT}/include)
  set(LIBCLANG_LIBRARY ${LLVM_ROOT}/lib/libclang.lib)
else ()
  message(STATUS "Building for Linux")
  target_compile_options(project_warnings INTERFACE
          -Wall
          -Werror
          -Wextra
          -Wshadow
          -Wnon-virtual-dtor
          -Wold-style-cast
          -Wcast-align
          -Wunused
          -Woverloaded-virtual
          -Wpedantic
          -Wconversion
          -Wsign-conversion
          -Wnull-dereference
          -Wdouble-promotion
          -Wformat=2)

  if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    target_compile_options(project_warnings INTERFACE
            -Wmisleading-indentation
            -Wduplicated-cond
            -Wduplicated-branches
            -Wlogical-op
            -Wuseless-cast)
  endif ()

  find_program(LLVM_CONFIG_EXE "llvm-config")

  if (LLVM_CONFIG_EXE)
    message(STATUS "Found llvm-config: ${LLVM_CONFIG_EXE}")
  else ()
    message(FATAL_ERROR "Could not find llvm-config")
  endif ()

  execute_process(COMMAND ${LLVM_CONFIG_EXE} "--includedir" OUTPUT_VARIABLE LIBCLANG_INCLUDE_DIR)
  if (LIBCLANG_INCLUDE_DIR)
    string(REGEX REPLACE "\n$" "" LIBCLANG_INCLUDE_DIR "${LIBCLANG_INCLUDE_DIR}")
  else ()
    message(FATAL_ERROR "Could not find libclang include dir")
  endif ()

  execute_process(COMMAND ${LLVM_CONFIG_EXE} "--libdir" OUTPUT_VARIABLE LIBCLANG_PATH)
  if (LIBCLANG_PATH)
    string(REGEX REPLACE "\n$" "" LIBCLANG_PATH "${LIBCLANG_PATH}")
    find_file(LIBCLANG_LIBRARY NAMES libclang.so PATHS ${LIBCLANG_PATH})
    if (NOT LIBCLANG_LIBRARY)
      message(FATAL_ERROR "Could not find libclang")
    endif ()
  else ()
    message(FATAL_ERROR "Could not find libclang library directory")
  endif ()
endif ()

message(STATUS "Found libclang include directory: ${LIBCLANG_INCLUDE_DIR}")
message(STATUS "Found libclang library: ${LIBCLANG_LIBRARY}")

include_directories(
        ${PROJECT_SOURCE_DIR}/src
        ${PROJECT_SOURCE_DIR}/ext/cxxopts/include
        ${PROJECT_SOURCE_DIR}/ext/termcolor/include
        ${LIBCLANG_INCLUDE_DIR})

add_subdirectory(src)
add_subdirectory(test)
