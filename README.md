# klang
Developer tools for C++ built over `libclang`.

## `cppgrep`
```text
$ cppgrep -h
Greps intelligently through C++ code
Usage:
  cppgrep [OPTION...] positional parameters

  -h, --help      Print usage
      --class     Grep for class declarations
      --struct    Grep for struct declarations
      --template  Grep for class/struct template declarations
      --function  Grep for function declarations
      --variable  Grep for variable/member/param declarations

$ cppgrep -vf repos/klang/src/cppgrep/cppgrep.cpp

"/home/myuser/repos/klang/src/cppgrep/cppgrep.cpp"
16:10 grep_classes [member]
17:10 grep_templates [member]
18:10 grep_structs [member]
19:10 grep_functions [member]
20:10 grep_variables [member]
21:27 files [member]
26:47 source [param]
39:10 visit_children [function] [template]
39:29 visitor [param]
41:16 CXChildVisitResult [variable]
43:14 cursor [variable]
53:13 _index [member]
54:23 _unit [member]
57:13 parse_args [function]
57:28 argc [param]
57:46 argv [param]
59:17 cli_opts [variable]
110:48  [param]
112:16 print_class [member]
113:16 print_template [member]
114:16 print_struct [member]
115:16 print_function [member]
116:16 print_variable [member]
118:37 cursor [param]
228:45 main [function]
232:17 opts [variable]
236:36 ex [variable]

"/home/myuser/repos/klang/src/cppgrep/cppgrep.hpp"
12:27 main [function]
12:36 argc [param]
12:54 argv [param]
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
