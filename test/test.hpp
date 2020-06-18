#ifndef KLANG_TEST_HPP
#define KLANG_TEST_HPP

#include <filesystem>

namespace fs = std::filesystem;

#ifndef RESOURCE_DIR
#error "Resource directory definition is missing"
#endif

const fs::path RESOURCES = fs::path { RESOURCE_DIR };

#endif
