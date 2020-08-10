file(GLOB resources ${CMAKE_CURRENT_SOURCE_DIR}/resources/*)
foreach (resource ${resources})
  file(COPY ${resource} DESTINATION ${CMAKE_BINARY_DIR}/resources/)
endforeach ()

add_definitions(-DRESOURCE_DIR="${CMAKE_BINARY_DIR}/resources")

add_executable(unit_tests cppgrep_test.cpp)
target_link_libraries(unit_tests PRIVATE project_warnings lib_cppgrep)
add_test(run_unit_tests unit_tests)
