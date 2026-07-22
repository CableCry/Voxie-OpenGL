#include "application.hpp"

#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <memory>

auto main() -> int {

    WindowPtr window;
    if (auto w = CreateWindow(1920, 1080); w.has_value()) {
        window = std::move(w).value();
    }

    glfwMakeContextCurrent(window.get());

    return 0;
}
