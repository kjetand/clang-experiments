#include "test.hpp"

#include <array>

#include <cppgrep/cppgrep.hpp>

int main()
{
    std::array<const char*, 2> argv { "cppgrep", "-h" };
    return klang::cppgrep::main(argv.size(), argv.data());
}
