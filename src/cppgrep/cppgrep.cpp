#include "cppgrep.hpp"

#include <filesystem>
#include <memory>

#include <clang-c/Index.h>
#include <cxxopts.hpp>
#include <termcolor/termcolor.hpp>

namespace fs = std::filesystem;

namespace klang::cppgrep {

struct cli_options {
    bool grep_classes { false };
    bool grep_templates { false };
    bool grep_structs { false };
    std::vector<fs::path> files;
};

cli_options parse_args(int argc, const char* argv[])
{
    cli_options cli_opts {};
    cxxopts::Options opts("cppgrep", "Greps intelligently through C++ code");
    opts.add_options()("h,help", "Print usage");
    opts.add_options()("c,class", "Grep for classes");
    opts.add_options()("t,template", "Grep for templates");
    opts.add_options()("s,struct", "Grep for structs");
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

    void operator()(const CXCursor& cursor) const noexcept
    {
        print_class(cursor);
        print_template(cursor);
        print_struct(cursor);
    }
};

void print_record_type(const CXCursor& cursor, std::string_view tag) noexcept
{
    auto location = get_line_info(cursor);
    std::cout << termcolor::blue << location.line << ":" << location.column << termcolor::reset;
    std::cout << " " << clang_getCString(clang_getCursorSpelling(cursor));
    std::cout << " " << termcolor::yellow << "[" << tag << "]" << termcolor::reset;
    std::cout << '\n';
}

void print_if_class(const CXCursor& cursor) noexcept
{
    if (clang_getCursorKind(cursor) == CXCursor_ClassDecl) {
        print_record_type(cursor, "class");
    }
}

void print_if_struct(const CXCursor& cursor) noexcept
{
    if (clang_getCursorKind(cursor) == CXCursor_StructDecl) {
        print_record_type(cursor, "struct");
    }
}

void print_if_template(const CXCursor& cursor) noexcept
{
    auto kind = clang_getCursorKind(cursor);
    if (kind == CXCursor_ClassTemplate) {
        print_record_type(cursor, "template");
    }
    if (kind == CXCursor_ClassTemplatePartialSpecialization) {
        print_record_type(cursor, "template partial specialization");
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
}

void grep(const fs::path& source, cli_options cli_opts)
{
    static cli_options opts;
    static printer print;

    opts = std::move(cli_opts);
    setup_printer(&print, opts);

    auto index = clang_createIndex(0, 0);
    auto unit = clang_parseTranslationUnit(index, source.string().c_str(), nullptr, 0, nullptr, 0, CXTranslationUnit_SingleFileParse);
    auto cursor = clang_getTranslationUnitCursor(unit);

    std::cout << termcolor::green << source << termcolor::reset << '\n';

    clang_visitChildren(
        cursor, [](auto cursor, auto, auto) {
            print(cursor);
            return CXChildVisit_Recurse;
        },
        nullptr);

    clang_disposeTranslationUnit(unit);
    clang_disposeIndex(index);
}

}

int klang::cppgrep::main(int argc, const char* argv[])
{
    cli_options opts;

    try {
        opts = parse_args(argc, argv);
    } catch (const std::exception& ex) {
        std::puts(ex.what());
        return 1;
    }

    for (const auto& source : opts.files) {
        if (fs::exists(source)) {
            grep(source, opts);
        } else {
            std::puts("File does not exist");
        }
    }
    return 0;
}
