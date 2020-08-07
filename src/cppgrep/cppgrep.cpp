#include "cppgrep.hpp"

#if _MSC_VER
#include <array>
#endif

#include <functional>
#include <optional>
#include <string_view>
#include <type_traits>
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
    std::string query {};
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
    CXIndex _index { nullptr };
    CXTranslationUnit _unit { nullptr };

public:
    explicit translation_unit(const fs::path& source) noexcept
        : _index(clang_createIndex(1, 0))
        , _unit(clang_parseTranslationUnit(_index, source.string().c_str(), CLANG_ARGS.data(), static_cast<int>(CLANG_ARGS.size()), nullptr, 0, 0))
    {
    }

    ~translation_unit() noexcept
    {
        clang_disposeTranslationUnit(_unit);
        clang_disposeIndex(_index);
    }

    translation_unit(const translation_unit&) noexcept = default;
    translation_unit& operator=(const translation_unit&) noexcept = default;
    translation_unit(translation_unit&&) noexcept = default;
    translation_unit& operator=(translation_unit&&) noexcept = default;

    template <typename F>
    void visit_children(F&& visitor) const noexcept
    {
        static CXChildVisitResult (*visit)(CXCursor) { nullptr };
        visit = visitor;
        auto cursor = clang_getTranslationUnitCursor(_unit);

        clang_visitChildren(
            cursor, [](auto c, auto /*unused*/, auto /*unused*/) {
                return visit(c);
            },
            nullptr);
    }
};

cli_options parse_args(std::span<char*> args)
{
    cli_options cli_opts {};
    cxxopts::Options opts("cppgrep", "Greps intelligently through C++ code");
    opts.add_options()("h,help", "Print usage");
    opts.add_options()("class", "Grep for class declarations only");
    opts.add_options()("struct", "Grep for struct declarations only");
    opts.add_options()("template", "Grep for class/struct template declarations only");
    opts.add_options()("function", "Grep for function declarations only");
    opts.add_options()("variable", "Grep for variable/member/param declarations only");
    opts.add_options()("q,query", "Grep query string", cxxopts::value<std::string>(cli_opts.query));
    opts.add_options()("positional", "Positional arguments", cxxopts::value<std::vector<fs::path>>(cli_opts.files));

    opts.parse_positional({ "query", "positional" });

    auto** argv = args.data();
    auto argc = static_cast<int>(args.size());
    const auto result = opts.parse(argc, argv);

    if (result.count("help") != 0U) {
        std::puts(opts.help().c_str());
    }
    if (result.count("query") == 0U || cli_opts.query.empty()) {
        throw std::runtime_error("Missing grep query");
    }
    if (cli_opts.files.empty()) {
        throw std::runtime_error("Missing at least one source input file");
    }
    if (result.count("class") != 0U) {
        cli_opts.grep_classes = true;
    }
    if (result.count("template") != 0U) {
        cli_opts.grep_templates = true;
    }
    if (result.count("struct") != 0U) {
        cli_opts.grep_structs = true;
    }
    if (result.count("function") != 0U) {
        cli_opts.grep_functions = true;
    }
    if (result.count("variable") != 0U) {
        cli_opts.grep_variables = true;
    }
    if (!cli_opts.at_least_one_enabled()) {
        cli_opts.enable_all();
    }
    return cli_opts;
}

template <typename Factory, typename... Args>
concept CXStringFactory = std::is_same_v<std::invoke_result_t<Factory, Args...>, CXString>;

template <auto Factory>
class string_owner {
    using factory_type = decltype(Factory);

    std::string _str;

public:
    template <typename... Args>
    explicit string_owner(Args&&... args) noexcept
        requires(std::invocable<factory_type, Args...>&& CXStringFactory<factory_type, Args...>)
    {
        auto cx_str = Factory(std::forward<Args>(args)...);
        _str = clang_getCString(cx_str);
        clang_disposeString(cx_str);
    }

    ~string_owner() noexcept = default;

    string_owner(const string_owner&) noexcept = default;
    string_owner& operator=(const string_owner&) noexcept = default;
    string_owner(string_owner&&) noexcept = default;
    string_owner& operator=(string_owner&&) noexcept = default;

    [[nodiscard]] std::string_view get() const noexcept
    {
        return _str;
    }
};

[[nodiscard]] auto get_line_info(const CXCursor& cursor) noexcept
{
    std::pair<unsigned, unsigned> result;
    clang_getSpellingLocation(clang_getCursorLocation(cursor), nullptr, &result.first, &result.second, nullptr);
    return result;
}

[[nodiscard]] std::optional<grep_entry> extract(const cli_options& opts, const CXCursor& cursor, std::vector<std::string> tags) noexcept
{
    grep_entry result;

    const string_owner<clang_getCursorSpelling> spelling(cursor);

    if (spelling.get().find(opts.query) == std::string::npos) {
        return {};
    }
    auto [line, column] = get_line_info(cursor);
    result.line = line;
    result.column = column;
    result.identifier = spelling.get();
    result.tags = std::move(tags);

    return result;
}

[[nodiscard]] std::optional<grep_entry> extract_if(const cli_options& opts, const CXCursor& cursor) noexcept
{
    auto kind = clang_getCursorKind(cursor);

    if (opts.grep_classes && kind == CXCursor_ClassDecl) {
        return extract(opts, cursor, { "class" });
    }
    if (opts.grep_templates) {
        if (kind == CXCursor_ClassTemplate) {
            return extract(opts, cursor, { "template" });
        }
        if (kind == CXCursor_ClassTemplatePartialSpecialization) {
            return extract(opts, cursor, { "template", "partial specialization" });
        }
    }
    if (opts.grep_structs && kind == CXCursor_StructDecl) {
        return extract(opts, cursor, { "struct" });
    }
    if (opts.grep_functions) {
        if (kind == CXCursor_FunctionDecl) {
            return extract(opts, cursor, { "function" });
        }
        if (kind == CXCursor_FunctionTemplate) {
            return extract(opts, cursor, { "function", "template" });
        }
        if (kind == CXCursor_ConversionFunction) {
            return extract(opts, cursor, { "conversion function" });
        }
    }
    if (opts.grep_variables) {
        if (kind == CXCursor_VarDecl) {
            return extract(opts, cursor, { "variable" });
        }
        if (kind == CXCursor_FieldDecl) {
            return extract(opts, cursor, { "member" });
        }
        if (kind == CXCursor_ParmDecl) {
            return extract(opts, cursor, { "param" });
        }
    }
    return {};
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
        if (auto location = clang_getCursorLocation(cursor); clang_Location_isInSystemHeader(location) != 0 || clang_Location_isFromMainFile(location) == 0) {
            return CXChildVisit_Continue;
        }
        if (auto entry = extract_if(opts, cursor); entry) {
            results.entries.emplace_back(std::move(*entry));
        }
        return CXChildVisit_Recurse;
    });
    if (results.entries.empty()) {
        return {};
    }
    return results;
}

void grep(const cli_options& opts, const std::function<void(const grep_result&)>& gather_results) noexcept
{
    std::vector<grep_result> results;

    for (const auto& source : opts.files) {
        if (fs::exists(source)) {
            auto result = grep(source, opts);

            if (result) {
                gather_results(*result);
            }
        } else {
            std::cout << termcolor::red << "error: " << termcolor::reset << "Could not open source file " << fs::absolute(source).generic_string() << '\n';
        }
    }
}

void print_grep_result(const grep_result& result) noexcept
{
    std::cout << termcolor::green << fs::absolute(result.source).generic_string() << termcolor::reset << '\n';

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
    std::cout << '\n';
}

result_type main(std::span<char*> args) noexcept
{
    using klang::cppgrep::result_type;

    cli_options opts;

    try {
        opts = parse_args(args);
    } catch (const std::exception& ex) {
        std::puts(ex.what());
        return result_type::parse_args_failure;
    }
    grep(opts, [](auto& result) {
        print_grep_result(result);
    });
    return result_type::success;
}

}
