#include "application.hpp"
#include "logging.hpp"
#include "mesh.hpp"

#include <array>
#include <cstddef>
#include <exception>
#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <utility>

auto main() -> int try {

    auto app = orDieExp(Application::create(), "Application::create");

    // pos3, color3, uv2
    //    const std::array<float, 24> TRIANGLE{
    //        -0.5F, -0.5F, +0.0F, 1.0F, 1.0F, 1.0F, 0.0F, 0.0F, // bottom-left
    //        +0.5F, -0.5F, +0.0F, 1.0F, 1.0F, 1.0F, 1.0F, 0.0F, // bottom-right
    //        +0.0F, +0.5F, +0.0F, 1.0F, 1.0F, 1.0F, 0.5F, 1.0F, // top
    //    };

    const std::array<float, 24> TRIANGLE_OF_GOON{
        +0.0F, -0.5F, +0.0F, 1.0F, 1.0F, 1.0F, 0.5F, 0.0F, // bottom-left
        +0.5F, +0.5F, +0.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F, // bottom-right
        -0.5F, +0.5F, +0.0F, 1.0F, 1.0F, 1.0F, 0.0F, 1.0F, // top
    };

    const std::array<Mesh::Attrib, 3> TRIANGLE_ATTRIB{
        Mesh::Attrib{.location = 0, .components = 3, .offset = 0},
        Mesh::Attrib{.location = 1, .components = 3, .offset = (3 * sizeof(float))},
        Mesh::Attrib{.location = 2, .components = 2, .offset = (6 * sizeof(float))},
    };

    const auto triangle = Mesh::create(TRIANGLE_OF_GOON, 8 * sizeof(float), TRIANGLE_ATTRIB);

    if (auto r = app.loadTexture(
            "wall", "texture.png",
            {.wrapS = GL_MIRRORED_REPEAT, .wrapT = GL_MIRRORED_REPEAT, .mipmaps = true});
        !r) {
        Log::error("loadTexture: {}", r.error());
    }

    app.shader("triangle").use();
    app.shader("triangle").set("tex", 0); // sampler reads texture unit 0

    while (!app.window.shouldClose()) {
        app.processInput();

        if (const auto res = app.syncViewport()) {
            app.shader("triangle").use();
            app.shader("triangle").set("resolution", res->first, res->second);
        }

        Window::clearScreen();

        app.shader("triangle").use();
        app.texture("wall").bind(0);
        triangle.draw();

        app.pollAndSwap();
    }

    return 0;

} catch (const std::exception& e) {
    Log::error("fatal: {}", e.what());
    return 1;
}
