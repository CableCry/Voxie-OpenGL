# Libraries

Everything below is already fetched, built, and linked into the `voxel`
target by `CMakeLists.txt`. Just `#include` the headers and use them.

| Library | Version | What it does | Vendored as |
|---------|---------|--------------|-------------|
| **GLFW** | 3.4 | Creates the window, the OpenGL context, and handles keyboard/mouse/gamepad input. | git submodule → `external/glfw` |
| **GLAD** | GL 4.6 core | Loads the OpenGL 4.6 function pointers at runtime (you *must* call `gladLoadGL` before any `gl*` call). | pre-generated, `external/glad` |
| **GLM** | 1.1.0 | Header-only math: vectors, matrices, quaternions — matches GLSL types/naming. | git submodule → `external/glm` |
| **stb_image** | single-header | Loads PNG/JPG/etc. into pixel buffers for textures. | vendored, `external/stb` |

## How to use each

### GLFW — window + input
```cpp
#include <GLFW/glfw3.h>
```
Link target: `glfw`. Order matters — include **GLAD before GLFW**.
Docs: https://www.glfw.org/docs/latest/

### GLAD — OpenGL loader
```cpp
#include <glad/gl.h>   // gives you every gl* function for 4.6 core
// after making a context current:
gladLoadGL((GLADloadfunc)glfwGetProcAddress);
```
Link target: `glad_gl_core_46`.
Regenerate for a different GL version:
```sh
python -m glad --api gl:core=4.6 --out-path external/glad c
```
(needs the `glad` package + `jinja2`; only needed if you change the version).
Docs: https://gen.glad.sh/  ·  https://github.com/Dav1dde/glad

### GLM — math
```cpp
#include <glm/glm.hpp>                  // vec2/3/4, mat4, ...
#include <glm/gtc/matrix_transform.hpp> // translate/rotate/scale/perspective
#include <glm/gtc/type_ptr.hpp>         // glm::value_ptr(mat) -> float* for glUniform
```
Link target: `glm::glm` (header-only, no runtime cost).
Docs: https://github.com/g-truc/glm/blob/master/manual.md

### stb_image — image loading
```cpp
#include "stb_image.h"
int w, h, channels;
unsigned char* px = stbi_load("assets/foo.png", &w, &h, &channels, 0);
// ... upload to a GL texture ...
stbi_image_free(px);
```
Link target: `stb`. The implementation is compiled once in
`external/stb/stb_image.cpp` — don't define `STB_IMAGE_IMPLEMENTATION` yourself.
Docs: https://github.com/nothings/stb/blob/master/stb_image.h

## Adding another library
- **Header-only or CMake-friendly** → add as a submodule under `external/`,
  `add_subdirectory(...)` (or an `INTERFACE` target), then add it to
  `target_link_libraries(voxel PRIVATE ...)`.
- **Single-header** (like stb) → drop it in `external/<name>/` and link.
