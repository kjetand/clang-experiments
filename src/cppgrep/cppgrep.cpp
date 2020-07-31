#include "cppgrep.hpp"

#include <array>
#include <optional>
#include <string_view>
#include <vector>

#include <clang-c/Index.h>
#include <cxxopts.hpp>
#include <termcolor/termcolor.hpp>

namespace fs = std::filesystem;

namespace klang::cppgrep {

struct cli_options {
    bool grep_classes { false };
    bool grep_templates { false };
    bool grep_structs { false };
    bool grep_functions { false };
    bool grep_variables { false };
    std::vector<fs::path> files;

    [[nodiscard]] bool at_least_one_enabled() const noexcept
    {
        return grep_classes || grep_templates || grep_structs || grep_functions || grep_variables;
    }

    void enable_all() noexcept
    {
        grep_classes = true;
        grep_templates = true;
        grep_structs = true;
        grep_functions = true;
        grep_variables = true;
    }
};

constexpr std::array CLANG_ARGS { "-std=c++17" };

class translation_unit {
public:
    explicit translation_unit(const fs::path& source) noexcept
        : _index(clang_createIndex(true, false))
        , _unit(clang_parseTranslationUnit(_index, source.string().c_str(), CLANG_ARGS.data(), static_cast<int>(CLANG_ARGS.size()), nullptr, 0, 0))
    {
    }

    ~translation_unit() noexcept
    {
        clang_disposeTranslationUnit(_unit);
        clang_disposeIndex(_index);
    }

    template <typename F>
    void visit_children(F&& visitor) const noexcept
    {
        static CXChildVisitResult (*visit)(CXCursor) { nullptr };
        visit = visitor;
        auto cursor = clang_getTranslationUnitCursor(_unit);

        clang_visitChildren(
            cursor, [](auto c, auto, auto) {
                return visit(c);
            },
            nullptr);
    }

private:
    CXIndex _index { nullptr };
    CXTranslationUnit _unit { nullptr };
};

cli_options parse_args(int argc, const char* argv[])
{
    cli_options cli_opts {};
    cxxopts::Options opts("cppgrep", "Greps intelligently through C++ code");
    opts.add_options()("h,help", "Print usage");
    opts.add_options()("class", "Grep for class declarations");
    opts.add_options()("struct", "Grep for struct declarations");
    opts.add_options()("template", "Grep for class/struct template declarations");
    opts.add_options()("function", "Grep for function declarations");
    opts.add_options()("variable", "Grep for variable/member/param declarations");
    opts.add_options()("positional", "Positional arguments", cxxopts::value<std::vector<fs::path>>(cli_opts.files));

    std::vector<std::string> positional { "positional" };
    opts.parse_positional(positional.begin(), positional.end());
    const auto result = opts.parse(argc, const_cast<char**&>(argv)); // :(

    if (result.count("help")) {
        std::puts(opts.help().c_str());
    }
    if (result.count("class")) {
        cli_opts.grep_classes = true;
    }
    if (result.count("template")) {
        cli_opts.grep_templates = true;
    }
    if (result.count("struct")) {
        cli_opts.grep_structs = true;
    }
    if (result.count("function")) {
        cli_opts.grep_functions = true;
    }
    if (result.count("variable")) {
        cli_opts.grep_variables = true;
    }
    if (!cli_opts.at_least_one_enabled()) {
        cli_opts.enable_all();
    }
    return cli_opts;
}

template <auto Constructor>
class cxstring_owner {
    CXString _str;

public:
    template <typename... Args>
    explicit cxstring_owner(Args&&... args) noexcept
        : _str(Constructor(std::forward<Args>(args)...))
    {
    }

    ~cxstring_owner() noexcept
    {
        clang_disposeString(_str);
    }

    [[nodiscard]] std::string get() const noexcept
    {
        return clang_getCString(_str);
    }
};

[[nodiscard]] auto get_line_info(const CXCursor& cursor) noexcept
{
    std::pair<unsigned, unsigned> result;
    clang_getSpellingLocation(clang_getCursorLocation(cursor), nullptr, &result.first, &result.second, nullptr);
    return result;
}

[[nodiscard]] grep_entry extract(const CXCursor& cursor, std::vector<std::string> tags) noexcept
{
    grep_entry result;

    auto [line, column] = get_line_info(cursor);
    result.line = line;
    result.column = column;
    result.identifier = cxstring_owner<clang_getCursorSpelling>(cursor).get();
    result.tags = std::move(tags);

    return result;
}

[[nodiscard]] std::optional<grep_entry> extract_if(const cli_options& opts, const CXCursor& cursor) noexcept
{
    auto kind = clang_getCursorKind(cursor);

    if (opts.grep_classes && kind == CXCursor_ClassDecl) {
        return extract(cursor, { "class" });
    }
    if (opts.grep_templates) {
        if (kind == CXCursor_ClassTemplate) {
            return extract(cursor, { "template" });
        }
        if (kind == CXCursor_ClassTemplatePartialSpecialization) {
            return extract(cursor, { "template", "partial specialization" });
        }
    }
    if (opts.grep_structs && kind == CXCursor_StructDecl) {
        return extract(cursor, { "struct" });
    }
    if (opts.grep_functions) {
        if (kind == CXCursor_FunctionDecl) {
            return extract(cursor, { "function" });
        }
        if (kind == CXCursor_FunctionTemplate) {
            return extract(cursor, { "function", "template" });
        }
        if (kind == CXCursor_ConversionFunction) {
            return extract(cursor, { "conversion function" });
        }
    }
    if (opts.grep_variables) {
        if (kind == CXCursor_VarDecl) {
            return extract(cursor, { "variable" });
        } else if (kind == CXCursor_FieldDecl) {
            return extract(cursor, { "member" });
        } else if (kind == CXCursor_ParmDecl) {
            return extract(cursor, { "param" });
        }
    }
    return std::nullopt;
}

[[nodiscard]] std::optional<grep_result> grep(const fs::path& source, cli_options cli_opts)
{
    static cli_options opts;
    static grep_result results;

    results = {};

    opts = std::move(cli_opts);
    results.source = source;

    translation_unit tu(source);
    tu.visit_children([](auto cursor) {
        if (auto location = clang_getCursorLocation(cursor); clang_Location_isInSystemHeader(location)) {
            return CXChildVisit_Continue;
        }
        if (auto entry = extract_if(opts, cursor); entry) {
            results.entries.emplace_back(std::move(*entry));
        }
        return CXChildVisit_Recurse;
    });
    if (results.entries.empty()) {
        return std::nullopt;
    }
    return results;
}

std::vector<grep_result> grep(const cli_options& opts) noexcept
{
    std::vector<grep_result> results;

    for (const auto& source : opts.files) {
        auto result = grep(source, opts);

        if (result) {
            results.emplace_back(std::move(*result));
        }
    }
    return results;
}

void print_grep_results(const std::vector<grep_result>& results) noexcept
{
    for (const auto& result : results) {
        std::cout << termcolor::green << result.source << termcolor::reset << '\n';

        for (const auto& entry : result.entries) {
            std::cout << termcolor::blue << entry.line << ':' << entry.column << termcolor::reset << ' ' << entry.identifier;

            if (!entry.tags.empty()) {
                std::cout << ' ';
            }
            for (const auto& tag : entry.tags) {
                std::cout << termcolor::yellow << '[' << tag << ']' << termcolor::reset << ' ';
            }
            std::cout << '\n';
        }
    }
}

result_type main(int argc, const char* argv[])
{
    using klang::cppgrep::result_type;

    cli_options opts;

    try {
        opts = parse_args(argc, argv);
    } catch (const std::exception& ex) {
        std::puts(ex.what());
        return result_type::parse_args_failure;
    }
    print_grep_results(grep(opts));
    return result_type::success;
}

}
