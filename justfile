profile := env_var_or_default("PROFILE", "Debug")
build_dir := env_var_or_default("BUILD_DIR", "build_" + profile)
cmake_args := env_var_or_default("CMAKE_ARGS", "")
ctest_args := env_var_or_default("CTEST_ARGS", "")

[private]
default:
    @just --list

configure:
    cmake -S . -B {{build_dir}} -G Ninja -DCMAKE_BUILD_TYPE={{profile}} -DCMAKE_EXPORT_COMPILE_COMMANDS=ON {{cmake_args}}

build: configure
    cmake --build {{build_dir}}

test: build
    ctest --test-dir {{build_dir}} --output-on-failure --no-tests=ignore --progress --verbose {{ctest_args}}

test-extra: build
    ctest --test-dir {{build_dir}} --output-on-failure --no-tests=ignore --progress -VV {{ctest_args}}

clean:
    rm -rf {{build_dir}}

format:
    files=$(git ls-files '*.cpp' '*.h' '*.hpp' '*.cc' '*.cxx'); if [ -n "$files" ]; then clang-format -i $files; fi

format-check:
    files=$(git ls-files '*.cpp' '*.h' '*.hpp' '*.cc' '*.cxx'); if [ -n "$files" ]; then clang-format --dry-run --Werror $files; fi

lint: configure
    cmake --build {{build_dir}}
