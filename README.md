# klang
Developer tools for C++ built over `libclang`.

## `cppgrep`
```text
$ cppgrep -h
Greps intelligently through C++ code
Usage:
  cppgrep [OPTION...] positional parameters

  -h, --help         Print usage
      --class        Grep for class declarations only
      --struct       Grep for struct declarations only
      --template     Grep for class/struct template declarations only
      --function     Grep for function declarations only
      --variable     Grep for variable/member/param declarations only
  -q, --query arg    Optional grep query string
  -i, --ignore-case  Ignore case when using grep queries

$ cppgrep cppgrep.cpp cppgrep.hpp
C:/Users/Kjetil Andresen/CLionProjects/klang/src/cppgrep/cppgrep.cpp
22:8 cli_options [struct]
23:10 grep_classes [member]
24:10 grep_templates [member]
25:10 grep_structs [member]
26:10 grep_functions [member]
27:10 grep_variables [member]
28:10 ignore_case [member]
29:17 query [member]
30:27 files [member]
47:22 CLANG_ARGS [variable]
49:7 translation_unit [class]
50:13 _index [member]
51:23 _unit [member]
54:47 source [param]
66:45  [param]
67:56  [param]
68:40  [param]
69:51  [param]
72:10 visit_children [function] [template]
72:29 visitor [param]
86:13 parse_args [variable]
140:7 string_owner [template]
143:17 _str [member]
147:14 string_owner<Factory> [function] [template]
147:37 args [param]
157:37  [param]
158:48  [param]
159:32  [param]
160:43  [param]
168:20 get_line_info [function]
168:50 cursor [param]
170:35 result [variable]
175:41 extract [function]
175:68 opts [param]
175:90 cursor [param]
175:123 tags [param]
177:16 result [variable]
179:49 spelling [variable]
184:16 find_substring [variable]
184:53 haystack [param]
184:80 needle [param]
184:99 ignore_case [param]
204:41 extract_if [function]
204:71 opts [param]
204:93 cursor [param]
206:10 kind [variable]
247:42 grep [function]
247:63 source [param]
247:83 cli_opts [param]
249:24 opts [variable]
250:24 results [variable]
257:22 tu [variable]
273:6 grep [function]
273:30 opts [param]
273:83 gather_results [param]
273:79  [param]
275:30 results [variable]
290:6 print_grep_result [function]
290:43 result [param]
308:13 main [variable]

C:/Users/Kjetil Andresen/CLionProjects/klang/src/cppgrep/cppgrep.hpp
11:8 cli_options [struct]
18:8 grep_entry [struct]
19:14 line [member]
20:14 column [member]
21:17 identifier [member]
22:30 tags [member]
25:8 grep_result [struct]
26:27 source [member]
27:29 entries [member]
30:40 grep [function]
30:64 opts [param]
32:27 main [variable]
```

Currently under development...

## Howto build
Clone git repo with `git clone --recursive https://github.com/kjetand/klang`.

### Windows
Make sure that `libclang` is installed (through LLVM package). You can find the latest pre-built binary under
https://releases.llvm.org/download.html. When installed, make sure that LLVM is in your user `PATH`
environment variable, usually located in `C:\Program Files\LLVM\bin`. Build with cmake:

```text
mkdir build
cd build
cmake .. -DLLVM_ROOT="C:\Program Files\LLVM"
cmake --build .
```

### Linux
Make sure that `libclang` is installed (e.g. on Ubuntu the package is named `libclang-dev`). The cmake script uses
`llvm-config` to find include directories and library paths. Install `llvm` in order to get `llvm-config`. Build source
with cmake:

```text
mkdir build
cd build
cmake ..
make
```

## License
Licensed under the [MIT License](LICENSE).
