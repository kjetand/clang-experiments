#include "test.hpp"

#include <cppgrep/cppgrep.hpp>

int main()
{
    const auto cpp_file = "\"" + (klang::test::get_resource_directory() / "people.cpp").generic_string() + "\"";
    std::array argv { "cppgrep", "a", cpp_file.c_str() };
    return static_cast<int>(klang::cppgrep::main(static_cast<int>(argv.size()), argv.data()));
}