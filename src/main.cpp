#include "application.hpp"
#include "logging.hpp"

#include <array>
#include <exception>
#include <glad/gl.h>
#include <GLFW/glfw3.h>

auto main() -> int try {

    auto app = orDieExp(Application::create(), "Application::create");

    const std::array<float, 18> TRIS{
        -0.75, -0.5, +0.0F, // 1 BL
        -0.25, -0.5, +0.0F, // 1 BR
        -0.5,  +0.5, +0.0F, // 1 T

        +0.75, -0.5, +0.0F, // 1 BL
        +0.25, -0.5, +0.0F, // 1 BR
        +0.5,  +0.5, +0.0F, // 1 T
    };

    GLuint VBO = 0;
    GLuint VAO = 0;

    glCreateVertexArrays(1, &VAO);
    glCreateBuffers(1, &VBO);

    // flags = 0: filled at creation, never written or mapped again
    glNamedBufferStorage(VBO, TRIS.size() * sizeof(float), TRIS.data(), GL_NONE);
    glEnableVertexArrayAttrib(VAO, 0);
    glVertexArrayAttribFormat(VAO, 0, 3, GL_FLOAT, GL_FALSE, 0);
    glVertexArrayAttribBinding(VAO, 0, 0);
    std::cout << "HERE 1 \n";

    glVertexArrayVertexBuffer(VAO, 0, VBO, 0, 3 * sizeof(float));
    std::cout << "HERE 2 \n";

    app.shader("triangle").use();
    std::cout << "HERE 3 \n";

    while (!app.window.shouldClose()) {
        app.processInput();
        Window::clearScreen();

        app.shader("triangle").use();
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, (TRIS.size() / 3) / 2);
        app.shader("triangle2").use();
        glDrawArrays(GL_TRIANGLES, 3, (TRIS.size() / 3) / 2);

        app.pollAndSwap();
    }

    return 0;

} catch (const std::exception& e) {
    Log::error("fatal: {}", e.what());
    return 1;
}
