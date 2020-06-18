#include "test.hpp"

#include <cppgrep/cppgrep.hpp>

#include <array>

auto PEOPLE_CPP = (RESOURCES / "people.cpp").string();
auto TEMPLATES_HPP = (RESOURCES / "templates.hpp").string();

int main()
{
    std::array<const char*, 5> argv { "cppgrep", "-c", "-t", PEOPLE_CPP.c_str(), TEMPLATES_HPP.c_str() };
    return klang::cppgrep::main(argv.size(), argv.data());
}