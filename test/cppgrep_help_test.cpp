#include <array>

#include <cppgrep/cppgrep.hpp>

int main()
{
    std::array<const char*, 2> argv { "cppgrep", "-h" };
    return static_cast<int>(klang::cppgrep::main({ argv.data(), argv.size() }));
}
