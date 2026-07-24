#include "application.hpp"
#include "logging.hpp"

#include <array>
#include <exception>
#include <glad/gl.h>
#include <GLFW/glfw3.h>

auto main() -> int try {

    auto app = orDieExp(Application::create(), "Application::create");

    const std::array<float, 12> SQUARE{
        -0.5F, -0.5F, +0.0F, // bottom-left
        +0.5F, +0.5F, +0.0F, // top-right
        -0.5F, +0.5F, +0.0F, // top-left
        +0.5F, -0.5F, +0.0F, // bottom-right
    };
    const std::array<GLuint, 6> SQUARE_INDEX{0, 1, 2, 0, 3, 1};

    const std::array<Mesh::Attrib, 1> attribs{
        Mesh::Attrib{.location = 0, .components = 3, .offset = 0},
    };
    const auto square = Mesh::create(SQUARE, SQUARE_INDEX, 3 * sizeof(float), attribs);

    app.shader("triangle").use();

    while (!app.window.shouldClose()) {
        app.processInput();
        Window::clearScreen();

        app.shader("triangle").use();
        square.draw();

        app.pollAndSwap();
    }

    return 0;

} catch (const std::exception& e) {
    Log::error("fatal: {}", e.what());
    return 1;
}
