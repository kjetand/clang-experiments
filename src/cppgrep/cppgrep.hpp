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
    struct cursor {
        unsigned line;
        unsigned column;
        std::string identifier;

        explicit cursor(const CXCursor& cursor) noexcept;
    };

    struct class_decl : cursor {
        explicit class_decl(const CXCursor& c) noexcept;
    };

    struct class_template : cursor {
        explicit class_template(const CXCursor& cursor) noexcept;
    };

    struct class_template_partial : cursor {
        explicit class_template_partial(const CXCursor& cursor) noexcept;
    };

    struct struct_decl : cursor {
        explicit struct_decl(const CXCursor& cursor) noexcept;
    };

    struct function_decl : cursor {
        explicit function_decl(const CXCursor& cursor) noexcept;
    };

    struct function_template : cursor {
        explicit function_template(const CXCursor& cursor) noexcept;
    };

    struct conversion_function : cursor {
        explicit conversion_function(const CXCursor& cursor) noexcept;
    };

    struct var_decl : cursor {
        explicit var_decl(const CXCursor& cursor) noexcept;
    };

    struct field_decl : cursor {
        explicit field_decl(const CXCursor& cursor) noexcept;
    };

    struct param_decl : cursor {
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
