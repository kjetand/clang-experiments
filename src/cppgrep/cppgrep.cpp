#include "cppgrep.hpp"

#include <filesystem>
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
};

class translation_unit {
public:
    explicit translation_unit(const fs::path& source) noexcept
        : _index(clang_createIndex(0, 0))
        , _unit(clang_parseTranslationUnit(_index, source.string().c_str(), nullptr, 0, nullptr, 0, CXTranslationUnit_SingleFileParse))
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
            cursor, [](auto cursor, auto, auto) {
                return visit(cursor);
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
    opts.add_options()("c,class", "Grep for class declarations");
    opts.add_options()("s,struct", "Grep for struct declarations");
    opts.add_options()("t,template", "Grep for class/struct template declarations");
    opts.add_options()("f,function", "Grep for function declarations");
    opts.add_options()("v,variable", "Grep for variable/member/param declarations");
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
    return cli_opts;
}

struct line_info {
    unsigned line;
    unsigned column;
};

[[nodiscard]] auto get_line_info(const CXCursor& cursor) noexcept -> line_info
{
    unsigned line;
    unsigned column;
    clang_getSpellingLocation(clang_getCursorLocation(cursor), nullptr, &line, &column, nullptr);
    return { line, column };
}

void print_nop(const CXCursor&) noexcept { }

struct printer {
    using print_type = void (*)(const CXCursor&);

    print_type print_class = &print_nop;
    print_type print_template = &print_nop;
    print_type print_struct = &print_nop;
    print_type print_function = &print_nop;
    print_type print_variable = &print_nop;

    void operator()(const CXCursor& cursor) const noexcept
    {
        print_class(cursor);
        print_template(cursor);
        print_struct(cursor);
        print_function(cursor);
        print_variable(cursor);
    }
};

void print_record_type(const CXCursor& cursor, const std::vector<std::string_view>& tags) noexcept
{
    auto location = get_line_info(cursor);
    std::cout << termcolor::blue << location.line << ":" << location.column << termcolor::reset;
    std::cout << " " << clang_getCString(clang_getCursorSpelling(cursor));
    for (auto tag : tags) {
        std::cout << " " << termcolor::yellow << "[" << tag << "]" << termcolor::reset;
    }
    std::cout << '\n';
}

void print_if_class(const CXCursor& cursor) noexcept
{
    if (clang_getCursorKind(cursor) == CXCursor_ClassDecl) {
        print_record_type(cursor, { "class" });
    }
}

void print_if_struct(const CXCursor& cursor) noexcept
{
    if (clang_getCursorKind(cursor) == CXCursor_StructDecl) {
        print_record_type(cursor, { "struct" });
    }
}

void print_if_template(const CXCursor& cursor) noexcept
{
    auto kind = clang_getCursorKind(cursor);
    if (kind == CXCursor_ClassTemplate) {
        print_record_type(cursor, { "template" });
    }
    if (kind == CXCursor_ClassTemplatePartialSpecialization) {
        print_record_type(cursor, { "template", "partial specialization" });
    }
}

void print_if_function(const CXCursor& cursor) noexcept
{
    auto kind = clang_getCursorKind(cursor);
    if (kind == CXCursor_FunctionDecl) {
        print_record_type(cursor, { "function" });
    }
    if (kind == CXCursor_FunctionTemplate) {
        print_record_type(cursor, { "function", "template" });
    }
    if (kind == CXCursor_ConversionFunction) {
        print_record_type(cursor, { "conversion function" });
    }
}

void print_if_variable(const CXCursor& cursor) noexcept
{
    auto kind = clang_getCursorKind(cursor);
    if (kind == CXCursor_VarDecl) {
        print_record_type(cursor, { "variable" });
    } else if (kind == CXCursor_FieldDecl) {
        print_record_type(cursor, { "member" });
    } else if (kind == CXCursor_ParmDecl) {
        print_record_type(cursor, { "param" });
    }
}

void setup_printer(printer* print, const cli_options& opts) noexcept
{
    if (opts.grep_classes) {
        print->print_class = &print_if_class;
    }
    if (opts.grep_templates) {
        print->print_template = &print_if_template;
    }
    if (opts.grep_structs) {
        print->print_struct = &print_if_struct;
    }
    if (opts.grep_functions) {
        print->print_function = &print_if_function;
    }
    if (opts.grep_variables) {
        print->print_variable = &print_if_variable;
    }
}

void grep(const fs::path& source, cli_options cli_opts)
{
    static cli_options opts;
    static printer print;

    opts = std::move(cli_opts);
    setup_printer(&print, opts);

    std::cout << termcolor::green << source << termcolor::reset << '\n';

    translation_unit tu(source);
    tu.visit_children([](auto cursor) {
        print(cursor);
        return CXChildVisit_Recurse;
    });
}

}

klang::cppgrep::result_type klang::cppgrep::main(int argc, const char* argv[])
{
    using klang::cppgrep::result_type;

    cli_options opts;

    try {
        opts = parse_args(argc, argv);
    } catch (const std::exception& ex) {
        std::puts(ex.what());
        return result_type::parse_args_failure;
    }

    for (const auto& source : opts.files) {
        if (!fs::exists(source)) {
            std::cout << "File " << source << " doesn't exist" << '\n';
            return result_type::file_not_found_failure;
        }
    }

    for (const auto& source : opts.files) {
        grep(source, opts);
    }
    return result_type::success;
}
