#include "test.hpp"

#include <array>

#include <cppgrep/cppgrep.hpp>

int main()
{
    using klang::cppgrep::result_type;
    std::array<const char*, 2> argv { "cppgrep", "/path/that/does/not/exist" };
    return klang::cppgrep::main(static_cast<int>(argv.size()), argv.data()) == result_type::file_not_found_failure ? 0 : 1;
}
