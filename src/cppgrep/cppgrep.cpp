#include "cppgrep.hpp"

#if _MSC_VER
#include <array>
#endif

#include <algorithm>
#include <optional>
#include <string_view>
#include <type_traits>
#include <unordered_map>

#include <cxxopts.hpp>
#include <termcolor/termcolor.hpp>

namespace fs = std::filesystem;

namespace klang::cppgrep {

template <class... Ts>
struct overloaded : Ts... {
    using Ts::operator()...;
};

template <class... Ts>
overloaded(Ts...) -> overloaded<Ts...>;

struct cli_options {
    bool grep_classes { false };
    bool grep_templates { false };
    bool grep_structs { false };
    bool grep_functions { false };
    bool grep_variables { false };
    bool ignore_case { false };
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

using parse_result = std::variant<std::monostate, cli_options>;

parse_result parse_args(std::span<const char*> args)
{
    cli_options cli_opts {};
    cxxopts::Options opts("cppgrep", "Greps intelligently through C++ code");
    opts.add_options()("h,help", "Print usage");
    opts.add_options()("class", "Grep for class declarations only");
    opts.add_options()("struct", "Grep for struct declarations only");
    opts.add_options()("template", "Grep for class/struct template declarations only");
    opts.add_options()("function", "Grep for function declarations only");
    opts.add_options()("variable", "Grep for variable/member/param declarations only");
    opts.add_options()("q,query", "Optional grep query string", cxxopts::value<std::string>(cli_opts.query));
    opts.add_options()("i,ignore-case", "Ignore case when using grep queries");
    opts.add_options()("positional", "Positional arguments", cxxopts::value<std::vector<fs::path>>(cli_opts.files));

    opts.parse_positional({ "positional" });

    auto** argv = args.data();
    auto argc = static_cast<int>(args.size());
    const auto result = opts.parse(argc, argv);

    if (result.count("help") != 0U) {
        std::puts(opts.help().c_str());
        return {};
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
    if (result.count("ignore-case") != 0U) {
        cli_opts.ignore_case = true;
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

[[nodiscard]] bool has_substring(std::string_view needle, std::string_view haystack, const bool ignore_case) noexcept
{
    if (ignore_case) {
        return std::search(haystack.begin(), haystack.end(), needle.begin(), needle.end(), [](auto c1, auto c2) noexcept {
            return std::toupper(c1) == std::toupper(c2);
        }) != haystack.end();
    }
    return haystack.find(needle) != std::string_view::npos;
}

[[nodiscard]] std::string get_spelling(const CXCursor& cursor) noexcept
{
    return std::string(string_owner<clang_getCursorSpelling>(cursor).get());
}

tag::cursor::cursor(const CXCursor& cursor) noexcept
{
    const auto [l, c] = get_line_info(cursor);
    line = l;
    column = c;
    identifier = get_spelling(cursor);
}

tag::class_decl::class_decl(const CXCursor& c) noexcept
    : cursor(c)
{
}

tag::class_template::class_template(const CXCursor& c) noexcept
    : cursor(c)
{
}

tag::class_template_partial::class_template_partial(const CXCursor& c) noexcept
    : cursor(c)
{
}

tag::struct_decl::struct_decl(const CXCursor& c) noexcept
    : cursor(c)
{
}

tag::function_decl::function_decl(const CXCursor& c) noexcept
    : cursor(c)
{
}

tag::function_template::function_template(const CXCursor& c) noexcept
    : cursor(c)
{
}

tag::conversion_function::conversion_function(const CXCursor& c) noexcept
    : cursor(c)
{
}

tag::var_decl::var_decl(const CXCursor& c) noexcept
    : cursor(c)
{
}

tag::field_decl::field_decl(const CXCursor& c) noexcept
    : cursor(c)
{
}

tag::param_decl::param_decl(const CXCursor& c) noexcept
    : cursor(c)
{
}

[[nodiscard]] std::optional<std::function<grep_entry(const CXCursor&)>> cursor_kind_to_factory(const CXCursorKind kind) noexcept
{
    static const std::unordered_map<CXCursorKind, std::function<grep_entry(const CXCursor&)>> entry_factories {
        { CXCursor_ClassDecl, [](const auto& c) { return tag::class_decl { c }; } },
        { CXCursor_ClassTemplate, [](const auto& c) { return tag::class_template { c }; } },
        { CXCursor_ClassTemplatePartialSpecialization, [](const auto& c) { return tag::class_template_partial { c }; } },
        { CXCursor_StructDecl, [](const auto& c) { return tag::struct_decl { c }; } },
        { CXCursor_FunctionDecl, [](const auto& c) { return tag::function_decl { c }; } },
        { CXCursor_FunctionTemplate, [](const auto& c) { return tag::function_template { c }; } },
        { CXCursor_ConversionFunction, [](const auto& c) { return tag::conversion_function { c }; } },
        { CXCursor_VarDecl, [](const auto& c) { return tag::var_decl { c }; } },
        { CXCursor_FieldDecl, [](const auto& c) { return tag::field_decl { c }; } },
        { CXCursor_ParmDecl, [](const auto& c) { return tag::param_decl { c }; } }
    };
    const auto factory = entry_factories.find(kind);

    if (factory == entry_factories.cend()) {
        return {};
    }
    return factory->second;
}

[[nodiscard]] std::optional<grep_entry> create_entry(const CXCursor& cursor) noexcept
{
    const auto factory = cursor_kind_to_factory(clang_getCursorKind(cursor));

    if (factory) {
        return (*factory)(cursor);
    }
    return {};
}

[[nodiscard]] const tag::cursor& cast_to_parent(const grep_entry& entry) noexcept
{
    return std::visit(overloaded { [](const auto& e) -> const tag::cursor& { return e; } }, entry);
}

[[nodiscard]] bool match_requested_type(const cli_options& opts, const grep_entry& entry) noexcept
{
    return std::visit(overloaded {
                          [&opts](const tag::class_decl& /*unused*/) { return opts.grep_classes; },
                          [&opts](const tag::class_template& /*unused*/) { return opts.grep_classes; },
                          [&opts](const tag::class_template_partial& /*unused*/) { return opts.grep_classes; },
                          [&opts](const tag::struct_decl& /*unused*/) { return opts.grep_structs; },
                          [&opts](const tag::function_decl& /*unused*/) { return opts.grep_functions; },
                          [&opts](const tag::function_template& /*unused*/) { return opts.grep_functions; },
                          [&opts](const tag::conversion_function& /*unused*/) { return opts.grep_functions; },
                          [&opts](const tag::var_decl& /*unused*/) { return opts.grep_variables; },
                          [&opts](const tag::field_decl& /*unused*/) { return opts.grep_variables; },
                          [&opts](const tag::param_decl& /*unused*/) { return opts.grep_variables; },
                      },
        entry);
}

[[nodiscard]] bool match_query(const cli_options& opts, const grep_entry& entry) noexcept
{
    return opts.query.empty() || has_substring(opts.query, cast_to_parent(entry).identifier, opts.ignore_case);
}

[[nodiscard]] std::optional<grep_entry> get_entry(const cli_options& opts, const CXCursor& cursor) noexcept
{
    auto entry = create_entry(cursor);

    if (!entry || !match_requested_type(opts, *entry) || !match_query(opts, *entry)) {
        return {};
    }
    return entry;
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
        if (auto entry = get_entry(opts, cursor); entry) {
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
        std::cout << termcolor::blue << cast_to_parent(entry).line << ':' << cast_to_parent(entry).column << termcolor::reset << ' ' << cast_to_parent(entry).identifier;
        std::cout << '\n';
    }
    std::cout << '\n';
}

result_type main(std::span<const char*> args) noexcept
{
    using klang::cppgrep::result_type;

    parse_result parsed_args;

    try {
        parsed_args = parse_args(args);
    } catch (const std::exception& ex) {
        std::puts(ex.what());
        return result_type::parse_args_failure;
    }
    overloaded grep_visitor {
        [](const cli_options& opts) {
            grep(opts, [](auto& result) {
                print_grep_result(result);
            });
            return result_type::success;
        },
        [](std::monostate /* unused */) {
            return result_type::success;
        }
    };
    return std::visit(grep_visitor, parsed_args);
}
}
