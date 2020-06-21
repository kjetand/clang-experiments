#include "test.hpp"

#include <cppgrep/cppgrep.hpp>

#include <array>

auto PEOPLE_CPP = (RESOURCES / "people.cpp").string();

int main()
{
    std::array argv { "cppgrep", "-cstfv", PEOPLE_CPP.c_str() };
    return klang::cppgrep::main(argv.size(), argv.data());
}