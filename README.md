# klang
Developer tools for C++ built over `libclang`.

## `cppgrep`
```text
Greps intelligently through C++ code
Usage:
  cppgrep [OPTION...] positional parameters

  -h, --help      Print usage
  -c, --class     Grep for class declarations
  -s, --struct    Grep for struct declarations
  -t, --template  Grep for class/struct template declarations
  -f, --function  Grep for function declarations
  -v, --variable  Grep for variable declarations
```

Currently under development...

## Howto build
Clone git repo with `git clone --recursive https://github.com/kjetand/klang`.

### Windows
Now, make sure that LLVM is installed. You can find the latest pre-built binary under
https://releases.llvm.org/download.html. When installed, make sure that LLVM is in your user `PATH`
environment variable, usually located in `C:\Program Files\LLVM\bin`. Build with `cmake`:

```text
mkdir build
cd build
cmake .. -DLLVM_ROOT="C:\Program Files\LLVM" -DCMAKE_BUILD_TYPE:STRING=Release
cmake --build . --config "Release"
```

## License
Licensed under the [MIT License](LICENSE).
