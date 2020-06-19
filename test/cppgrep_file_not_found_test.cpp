#include "test.hpp"

#include <array>

#include <cppgrep/cppgrep.hpp>

int main()
{
    std::array<const char*, 2> argv { "cppgrep", "/path/that/does/not/exist" };
    return klang::cppgrep::main(argv.size(), argv.data()) == 2 ? 0 : 1;
}
