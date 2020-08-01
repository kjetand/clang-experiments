#if _MSC_VER
#include <array>
#endif

#include <cppgrep/cppgrep.hpp>

int main()
{
    std::array argv { "cppgrep", "-h" };
    return static_cast<int>(klang::cppgrep::main({ const_cast<char**>(argv.data()), argv.size() })); // NOLINT
}
