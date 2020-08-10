#include "cppgrep.hpp"

int main(int argc, const char* argv[])
{
    return static_cast<int>(klang::cppgrep::main({ argv, static_cast<std::size_t>(argc) }));
}