#include "test.hpp"

#if _MSC_VER
#include <array>
#endif

#include <cppgrep/cppgrep.hpp>

int main()
{
    const auto cpp_file = "\"" + (klang::test::get_resource_directory() / "people.cpp").generic_string() + "\"";
    const std::array argv { "cppgrep", "-q", "a", cpp_file.c_str() };
    return static_cast<int>(klang::cppgrep::main({ const_cast<char**>(argv.data()), argv.size() })); // NOLINT
}