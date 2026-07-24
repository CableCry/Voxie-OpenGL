#include "application.hpp"
#include "logging.hpp"

#include <array>
#include <exception>
#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <strings.h>

struct VAOData {
    GLuint vao;
    GLuint location;
    GLint stride;
    GLenum type;
    GLboolean isNormalize;
    GLuint offset;
};

auto main() -> int try {

    auto app = orDieExp(Application::create(), "Application::create");

    const std::array<float, 12> SQUARE{
        -0.5F, -0.5F, +0.0F, // bottom-left
        +0.5F, +0.5F, +0.0F, // top-right
        -0.5F, +0.5F, +0.0F, // top-left
        +0.5F, -0.5F, +0.0F, // bottom-right
    };
    const std::array<GLuint, 6> SQUARE_INDEX{0, 1, 2, 0, 3, 1};

    const GLsizeiptr pointsSize = SQUARE.size() * sizeof(float);
    const GLsizeiptr indexSize = SQUARE_INDEX.size() * sizeof(GLuint);
    const GLsizeiptr buffSize = pointsSize + indexSize;

    const GLintptr pointsOffset = 0;
    const GLintptr indexOffset = pointsSize; // Indices start right after points

    VAOData VAO{.vao = 0,
                .location = 0,
                .stride = 3,
                .type = GL_FLOAT,
                .isNormalize = GL_FALSE,
                .offset = 0};

    GLuint VBO = 0;

    glCreateVertexArrays(1, &VAO.vao);
    glCreateBuffers(1, &VBO);

    glNamedBufferStorage(VBO, buffSize, nullptr, GL_DYNAMIC_STORAGE_BIT);
    glNamedBufferSubData(VBO, pointsOffset, pointsSize, SQUARE.data());
    glNamedBufferSubData(VBO, pointsSize, indexSize, SQUARE_INDEX.data());

    glEnableVertexArrayAttrib(VAO.vao, VAO.location);
    glVertexArrayAttribFormat(VAO.vao, VAO.location, VAO.stride, VAO.type, VAO.isNormalize,
                              VAO.offset);
    glVertexArrayAttribBinding(VAO.vao, VAO.location, 0);

    glVertexArrayVertexBuffer(VAO.vao, 0, VBO, pointsOffset, 3 * sizeof(float));
    glVertexArrayElementBuffer(VAO.vao, VBO);

    app.shader.use();

    while (!app.window.shouldClose()) {
        app.processInput();
        Window::clearScreen();

        app.shader.use();
        glBindVertexArray(VAO.vao);
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(SQUARE_INDEX.size()), GL_UNSIGNED_INT,
                       reinterpret_cast<void*>(indexOffset));

        app.pollAndSwap();
    }

    glDeleteVertexArrays(1, &VAO.vao);
    glDeleteBuffers(1, &VBO);

    return 0;

} catch (const std::exception& e) {
    Log::error("fatal: {}", e.what());
    return 1;
}
