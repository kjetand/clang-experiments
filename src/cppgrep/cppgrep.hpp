#ifndef KLANG_CPPGREP_HPP
#define KLANG_CPPGREP_HPP

namespace klang::cppgrep {

enum class result_type : int {
    success = 0,
    parse_args_failure,
    file_not_found_failure
};

[[nodiscard]] result_type main(int argc, const char* argv[]);

}

#endif
