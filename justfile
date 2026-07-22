# Run `just` with no args to build + run.
set windows-shell := ["cmd.exe", "/c"]

exe := if os_family() == "windows" { "build\\voxel.exe" } else { "build/voxel" }

default: run

alias init := setup

# One-shot fresh-clone setup: pull everything the build needs (glfw, glm submodules).
setup:
    git submodule update --init --recursive

# Configure the build (Ninja single-config -> uniform binary path).
configure:
    cmake -B build -G Ninja

# Compile.
build: configure
    cmake --build build

# Build then launch.
run: build
    {{exe}}

# Remove the build dir.
clean:
    cmake -E rm -rf build

# Nuke and rebuild.
rebuild: clean build

# Format the code you write (src/ only), in place.
format:
    #!/usr/bin/env bash
    set -euo pipefail
    find src -type f \( -name '*.cpp' -o -name '*.hpp' -o -name '*.h' \) -print0 \
      | xargs -0 --no-run-if-empty clang-format -i

# Lint the code you write (src/ only). Add `-- --fix` to auto-apply fixes.
tidy *ARGS: configure
    #!/usr/bin/env bash
    set -euo pipefail
    find src -type f \( -name '*.cpp' -o -name '*.hpp' -o -name '*.h' \) -print0 \
      | xargs -0 --no-run-if-empty clang-tidy -p build {{ARGS}}
