#ifndef APPLICATION_H
#define APPLICATION_H

#include <cstdint>
#include <expected>
#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <memory>

struct GLFWwindowDeleter {
    void operator()(GLFWwindow* w) const noexcept;
};

using WindowPtr = std::unique_ptr<GLFWwindow, GLFWwindowDeleter>;

enum class GLFWerror : uint8_t {
    FAILED_TO_CREATE_WINDOW,
};

auto GetGLFWerrorText(GLFWerror e) -> std::string;
auto CreateWindow(int32_t w, int32_t h) -> std::expected<WindowPtr, GLFWerror>;

#endif // APPLICATION_H
