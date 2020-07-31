#ifndef KLANG_CPPGREP_HPP
#define KLANG_CPPGREP_HPP

#include <filesystem>
#include <string>
#include <vector>

namespace klang::cppgrep {

struct cli_options;

enum class result_type : int {
    success = 0,
    parse_args_failure,
    file_not_found_failure
};

struct grep_entry {
    unsigned line;
    unsigned column;
    std::string identifier;
    std::vector<std::string> tags;
};

struct grep_result {
    std::filesystem::path source;
    std::vector<grep_entry> entries;
};

[[nodiscard]] std::vector<grep_result> grep(const cli_options& opts) noexcept;

[[nodiscard]] result_type main(int argc, const char* argv[]);

}

#endif
