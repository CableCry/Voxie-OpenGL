# voxel

OpenGL 4.6 / C++23 starter. GLFW window + GLAD loader + GLM math + stb_image.

## Requirements
- CMake ≥ 3.24, Ninja, a C++23 compiler (GCC 13+, Clang 16+, or MSVC 2022).
- [`just`](https://github.com/casey/just) for the shortcuts below.
- On Windows: run from a *Visual Studio Developer* prompt (gives you MSVC + Ninja).

## Quick start
```sh
just setup     # fresh clone: pull submodules (glfw, glm)
just build     # configure + compile
just run       # build + launch
```

## Recipes
`just setup` · `just configure` · `just build` · `just run` · `just clean` · `just rebuild`
`just format` · `just tidy` (both act on `src/` only) — `just tidy -- --fix` auto-applies fixes.

## Editor (Zed)
`.clangd` points clangd at `build/compile_commands.json`, so run `just configure`
once and Zed gets full completion, diagnostics, and format-on-save. The
`.clang-format` / `.clang-tidy` configs live in `src/`, so they only affect your
code — `external/` is never linted or reformatted.

## Layout
```
src/       app code
shaders/   GLSL (loaded from disk at runtime via SHADER_DIR)
assets/    textures / models
external/  glfw, glm, glad (submodules), stb (vendored)
```
