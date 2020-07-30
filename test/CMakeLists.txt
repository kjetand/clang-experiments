enable_testing()

file(GLOB resources ${CMAKE_CURRENT_SOURCE_DIR}/resources/*)
foreach (resource ${resources})
  file(COPY ${resource} DESTINATION ${CMAKE_BINARY_DIR}/resources/)
endforeach ()

add_definitions(-DRESOURCE_DIR="${CMAKE_BINARY_DIR}/resources")

function(define_cppgrep_test name)
  add_executable(${name}_test ${CMAKE_CURRENT_SOURCE_DIR}/${name}_test.cpp)
  target_link_libraries(${name}_test PRIVATE lib_cppgrep)
  add_test(${name} ${name}_test)
endfunction()

define_cppgrep_test(cppgrep_help)
define_cppgrep_test(cppgrep_all)
define_cppgrep_test(cppgrep_file_not_found)
