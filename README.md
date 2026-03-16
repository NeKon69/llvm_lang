# llvm_lang

Quick start for building the repo locally.

## Requirements

- CMake 3.20+
- Ninja
- A C++ compiler with C++23 support
- LLVM development files
- Optional: `clang-tidy`, `clang-format`, `ccache`, `just`

## Linux (Ubuntu / Debian)

Install dependencies:

```bash
sudo apt-get update
sudo apt-get install -y ccache clang clang-format clang-tidy cmake llvm-dev ninja-build gcc-14 g++-14
```

Build:

```bash
export LLVM_DIR="$(llvm-config --cmakedir)"

cmake -S . -B build_Debug -G Ninja   -DCMAKE_BUILD_TYPE=Debug   -DCMAKE_C_COMPILER=gcc-14   -DCMAKE_CXX_COMPILER=g++-14   -DLLVM_DIR="${LLVM_DIR}"   -DLLVM_LANG_ENABLE_CLANG_TIDY=ON

cmake --build build_Debug
```

Test:

```bash
ctest --test-dir build_Debug --output-on-failure --no-tests=ignore --progress --verbose
```

## macOS

Install dependencies:

```bash
brew install ccache llvm ninja cmake
```

Build:

```bash
export PATH="$(brew --prefix llvm)/bin:$PATH"
export LLVM_DIR="$(brew --prefix llvm)/lib/cmake/llvm"

cmake -S . -B build_Debug -G Ninja   -DCMAKE_BUILD_TYPE=Debug   -DLLVM_DIR="${LLVM_DIR}"   -DLLVM_LANG_ENABLE_CLANG_TIDY=ON

cmake --build build_Debug
```

Test:

```bash
ctest --test-dir build_Debug --output-on-failure --no-tests=ignore --progress --verbose
```

## Windows

Windows CI builds with LLVM integration disabled, so this is the fastest supported local setup.

Install tools:

- Visual Studio 2022 Build Tools or Visual Studio 2022 with C++ workload
- CMake
- Ninja
- LLVM/Clang (recommended for `clang-format` and `clang-tidy`)

If you use Chocolatey:

```powershell
choco install -y cmake ninja llvm
```

Build from a Developer PowerShell for Visual Studio:

```powershell
cmake -S . -B build_Debug -G Ninja ^
  -DCMAKE_BUILD_TYPE=Debug ^
  -DLLVM_LANG_ENABLE_LLVM=OFF ^
  -DLLVM_LANG_ENABLE_CLANG_TIDY=ON

cmake --build build_Debug
```

Test:

```powershell
ctest --test-dir build_Debug --output-on-failure --no-tests=ignore --progress --verbose
```

## Format check

```bash
files=$(git ls-files '*.cpp' '*.h' '*.hpp' '*.cc' '*.cxx')
if [ -n "$files" ]; then
  clang-format --dry-run --Werror $files
fi
```

On PowerShell:

```powershell
$files = git ls-files '*.cpp' '*.h' '*.hpp' '*.cc' '*.cxx'
if ($files) {
  clang-format --dry-run --Werror $files
}
```

## Optional just commands

If you have `just` installed:

```bash
just build
just test
just lint
just format-check
```

By default these use the `Debug` profile and `build_Debug`.

## Notes

- If LLVM is installed in a non-standard location on Linux or macOS, set `LLVM_DIR` to the directory containing `LLVMConfig.cmake`.
- In-source builds are not supported.
