cmake_minimum_required(VERSION 3.24)
project(voxel LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "" FORCE)
endif()

# --- Dependencies -----------------------------------------------------------
# GLFW: window + context + input (git submodule)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(external/glfw)

# GLM: header-only math (git submodule)
add_subdirectory(external/glm)

# GLAD: OpenGL 4.6 core loader, pre-generated and vendored (external/glad).
# Regenerate with: python -m glad --api gl:core=4.6 --out-path external/glad c
add_library(glad_gl_core_46 STATIC external/glad/src/gl.c)
target_include_directories(glad_gl_core_46 PUBLIC external/glad/include)

# stb: single-header image loader (vendored). One TU emits the implementation.
add_library(stb STATIC external/stb/stb_image.cpp)
target_include_directories(stb PUBLIC external/stb)

# --- App --------------------------------------------------------------------
add_executable(voxel src/main.cpp src/application.cpp)
target_link_libraries(voxel PRIVATE glfw glm::glm glad_gl_core_46 stb)

# Point the app at the shader source dir (dev convenience; no copy step).
# ponytail: switch to copying shaders next to the binary when you ship a build.
target_compile_definitions(voxel PRIVATE
  SHADER_DIR="${CMAKE_CURRENT_SOURCE_DIR}/shaders")

if(WIN32)
  target_link_libraries(voxel PRIVATE opengl32)
elseif(UNIX)
  find_package(OpenGL REQUIRED)
  target_link_libraries(voxel PRIVATE OpenGL::GL ${CMAKE_DL_LIBS})
endif()

if(MSVC)
  target_compile_options(voxel PRIVATE /W4)
else()
  target_compile_options(voxel PRIVATE -Wall -Wextra)
endif()
