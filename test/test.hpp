#ifndef KLANG_TEST_HPP
#define KLANG_TEST_HPP

#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

#include <filesystem>

namespace fs = std::filesystem;

#ifndef RESOURCE_DIR
#error "Resource directory definition is missing"
#endif

namespace klang::test {

[[nodiscard]] inline fs::path get_resource_directory()
{
    return RESOURCE_DIR;
}

}

#endif
