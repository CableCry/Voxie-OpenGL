#include "application.hpp"
#include "logging.hpp"
#include "mesh.hpp"

#include <array>
#include <exception>
#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <utility>

auto main() -> int try {

    auto app = orDieExp(Application::create(), "Application::create");

    const std::array<float, 24> SQUARE{
        -0.5F, -0.5F, +0.0F, 1.0F, 0.0F, 0.0F, // bottom-left
        +0.5F, +0.5F, +0.0F, 0.0F, 1.0F, 0.0F, // top-right
        -0.5F, +0.5F, +0.0F, 0.0F, 0.0F, 1.0F, // top-left
        +0.5F, -0.5F, +0.0F, 1.0F, 0.0F, 0.0F, // bottom-right
    };
    const std::array<GLuint, 6> SQUARE_INDEX{0, 2, 4, 0, 6, 2};

    const std::array<Mesh::Attrib, 2> attribs{
        Mesh::Attrib{.location = 0, .components = 3, .offset = 0},                    // Points
        Mesh::Attrib{.location = 1, .components = 3, .offset = (3 * sizeof(float))}}; // Color

    const auto square = Mesh::create(SQUARE, SQUARE_INDEX, 3 * sizeof(float), attribs);

    std::pair<int, int> lastSize{}; // {0,0} => first frame always syncs

    while (!app.window.shouldClose()) {
        app.processInput();

        // minimized windows report 0x0; a 0 resolution NaNs the aspect divide in the shader
        if (const auto size = app.window.framebufferSize();
            size != lastSize && size.first > 0 && size.second > 0) {
            lastSize = size;
            glViewport(0, 0, size.first, size.second);
            app.shader("triangle").use();
            app.shader("triangle")
                .set("resolution", static_cast<float>(size.first), static_cast<float>(size.second));
        }

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
