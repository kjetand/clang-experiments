#ifndef KLANG_CPPGREP_HPP
#define KLANG_CPPGREP_HPP

#include <filesystem>
#include <functional>
#include <span>
#include <string>
#include <vector>
#include <variant>

#include <clang-c/Index.h>

namespace klang::cppgrep {

struct cli_options;

enum class result_type : int {
    success = 0,
    parse_args_failure
};

namespace tag {
    struct common_entry {
        unsigned line;
        unsigned column;
        std::string identifier;

        explicit common_entry(const CXCursor& cursor) noexcept;
    };

    struct class_decl : common_entry {
        explicit class_decl(const CXCursor& cursor) noexcept;
    };

    struct class_template : common_entry {
        explicit class_template(const CXCursor& cursor) noexcept;
    };

    struct class_template_partial : common_entry {
        explicit class_template_partial(const CXCursor& cursor) noexcept;
    };

    struct struct_decl : common_entry {
        explicit struct_decl(const CXCursor& cursor) noexcept;
    };

    struct function_decl : common_entry {
        explicit function_decl(const CXCursor& cursor) noexcept;
    };

    struct function_template : common_entry {
        explicit function_template(const CXCursor& cursor) noexcept;
    };

    struct conversion_function : common_entry {
        explicit conversion_function(const CXCursor& cursor) noexcept;
    };

    struct var_decl : common_entry {
        explicit var_decl(const CXCursor& cursor) noexcept;
    };

    struct field_decl : common_entry {
        explicit field_decl(const CXCursor& cursor) noexcept;
    };

    struct param_decl : common_entry {
        explicit param_decl(const CXCursor& cursor) noexcept;
    };

}

using grep_entry = std::variant<
    tag::class_decl,
    tag::class_template,
    tag::class_template_partial,
    tag::struct_decl,
    tag::function_decl,
    tag::function_template,
    tag::conversion_function,
    tag::var_decl,
    tag::field_decl,
    tag::param_decl>;

struct grep_result {
    std::filesystem::path source;
    std::vector<grep_entry> entries;
};

void grep(const cli_options& opts, const std::function<void(const grep_result&)>& gather_results) noexcept;

[[nodiscard]] result_type main(std::span<const char*> args) noexcept;

}

#endif
