#include "test.hpp"
#include "cppgrep/cppgrep.hpp"

#include <array>

TEST_CASE("Help returns success code", "[cppgrep]")
{
    using klang::cppgrep::result_type;
    std::array argv_short { "cppgrep", "-h" };
    std::array argv_long { "cppgrep", "--help" };
    REQUIRE(klang::cppgrep::main({ argv_short.data(), argv_short.size() }) == result_type::success);
    REQUIRE(klang::cppgrep::main({ argv_long.data(), argv_long.size() }) == result_type::success);
}