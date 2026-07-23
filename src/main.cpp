#include "application.hpp"
#include "logging.hpp"

#include <array>
#include <exception>
#include <glad/gl.h>
#include <GLFW/glfw3.h>

auto main() -> int try {

    auto app = orDieExp(Application::create(), "Application::create");

    const std::array<float, 9> TRIANGLE{
        -0.5F, -0.5F, +0.0F, // bottom-left
        +0.5F, -0.5F, +0.0F, // bottom-right
        +0.0F, +0.5F, +0.0F, // top-centre
    };

    GLuint VAO = 0;
    GLuint VBO = 0;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, TRIANGLE.size() * sizeof(float), TRIANGLE.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);

    app.shader.use();

    while (!app.window.shouldClose()) {
        app.processInput();
        Window::clearScreen();

        app.shader.use();
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        app.pollAndSwap();
    }

    glDeleteBuffers(1, &VBO);
    glDeleteVertexArrays(1, &VAO);

    return 0;

} catch (const std::exception& e) {
    Log::error("fatal: {}", e.what());
    return 1;
}
