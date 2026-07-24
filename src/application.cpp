#include "application.hpp"

#include "glad/gl.h"
#include "GLFW/glfw3.h"
#include "logging.hpp"
#include "shader.hpp"

#include <cassert>
#include <expected>
#include <string>
#include <string_view>
#include <utility>

[[nodiscard]] auto GlfwSession::create() -> std::expected<GlfwSession, std::string> {
    glfwSetErrorCallback(glfwErrorCallback);

    if (glfwInit() == GLFW_FALSE) {
        return std::unexpected("glfwInit() failed");
    }

    return GlfwSession{};
}

void GlfwSession::glfwErrorCallback(int code, const char* description) {
    Log::error("GLFW {} : {}", code, description);
}

GlfwSession::~GlfwSession() {
    if (owns_) {
        glfwTerminate();
    }
}

auto GlfwSession::operator=(GlfwSession&& o) noexcept -> GlfwSession& {
    std::swap(owns_, o.owns_);
    return *this;
}

void Window::GLFWwindowDeleter::operator()(GLFWwindow* w) const noexcept {
    glfwDestroyWindow(w);
}

[[nodiscard]] auto Window::create(int w, int h, const char* title)
    -> std::expected<Window, std::string> {
    auto session = GlfwSession::create();
    if (!session.has_value()) {
        return std::unexpected{std::move(session.error())};
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifndef NDEBUG
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
#endif
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
#endif

    Window::WindowPtr handle{glfwCreateWindow(w, h, title, nullptr, nullptr)};
    if (!handle) {
        return std::unexpected{"glfwCreateWindow() failed"};
    }

    glfwSetFramebufferSizeCallback(handle.get(), Window::onResizeCallback);

    Log::ok("GLFW {}", glfwGetVersionString());

    return Window{std::move(*session), std::move(handle)};
}

[[nodiscard]] auto Window::handle() noexcept -> GLFWwindow* {
    return window_.get();
}
[[nodiscard]] auto Window::handle() const noexcept -> const GLFWwindow* {
    return window_.get();
}

[[nodiscard]] auto Window::shouldClose() const noexcept -> bool {
    return glfwWindowShouldClose(window_.get()) == GLFW_TRUE;
}

void Window::requestClose() noexcept {
    glfwSetWindowShouldClose(window_.get(), GLFW_TRUE);
}

void Window::onResizeCallback([[maybe_unused]] GLFWwindow* _window, int width, int height) {
    glViewport(0, 0, width, height);
}

auto Window::isKeyPressed(int key) const noexcept -> bool {
    return glfwGetKey(window_.get(), key) == GLFW_PRESS;
}

void Window::swapBuffers() noexcept {
    glfwSwapBuffers(window_.get());
}

void Window::clearScreen() noexcept {
    glClearColor(0.2F, 0.3F, 0.3F, 1.0F);
    glClear(GL_COLOR_BUFFER_BIT);
}

auto Mesh::create(std::span<const float> vertices, std::span<const GLuint> indices,
                  GLsizei vertexStride, std::span<const Mesh::Attrib> attribs) -> Mesh {
    const GLsizeiptr vBytes = static_cast<GLsizeiptr>(vertices.size_bytes());
    const GLsizeiptr iBytes = static_cast<GLsizeiptr>(indices.size_bytes());

    Mesh mesh;
    mesh.indexCount_ = static_cast<GLsizei>(indices.size());
    mesh.indexOffset_ = vBytes; // indices packed right after vertices

    glCreateVertexArrays(1, &mesh.vao_);
    glCreateBuffers(1, &mesh.vbo_);

    glNamedBufferStorage(mesh.vbo_, vBytes + iBytes, nullptr, GL_DYNAMIC_STORAGE_BIT);
    glNamedBufferSubData(mesh.vbo_, 0, vBytes, vertices.data());
    glNamedBufferSubData(mesh.vbo_, vBytes, iBytes, indices.data());

    for (const auto& a : attribs) {
        glEnableVertexArrayAttrib(mesh.vao_, a.location);
        glVertexArrayAttribFormat(mesh.vao_, a.location, a.components, a.type, a.normalize,
                                  a.offset);
        glVertexArrayAttribBinding(mesh.vao_, a.location, 0);
    }

    glVertexArrayVertexBuffer(mesh.vao_, 0, mesh.vbo_, 0, vertexStride);
    glVertexArrayElementBuffer(mesh.vao_, mesh.vbo_);

    return mesh;
}

void Mesh::draw(GLenum mode) const {
    glBindVertexArray(vao_);
    glDrawElements(mode, indexCount_, GL_UNSIGNED_INT, reinterpret_cast<void*>(indexOffset_));
}

Mesh::~Mesh() {
    glDeleteVertexArrays(1, &vao_); // 0 is silently ignored by GL
    glDeleteBuffers(1, &vbo_);
}

Mesh::Mesh(Mesh&& o) noexcept
    : vao_{std::exchange(o.vao_, 0)}, vbo_{std::exchange(o.vbo_, 0)},
      indexCount_{std::exchange(o.indexCount_, 0)}, indexOffset_{std::exchange(o.indexOffset_, 0)} {
}

auto Mesh::operator=(Mesh&& o) noexcept -> Mesh& {
    std::swap(vao_, o.vao_);
    std::swap(vbo_, o.vbo_);
    std::swap(indexCount_, o.indexCount_);
    std::swap(indexOffset_, o.indexOffset_);
    return *this;
}

namespace {

// GL reports asynchronously; without this every failed call is silent.
void GLAD_API_PTR glDebugCallback([[maybe_unused]] GLenum source, [[maybe_unused]] GLenum type,
                                  GLuint id, GLenum severity, GLsizei length, const GLchar* message,
                                  [[maybe_unused]] const void* user) {
    const std::string_view msg{message, static_cast<std::size_t>(length)};
    if (severity == GL_DEBUG_SEVERITY_HIGH) {
        Log::error("GL [{}] {}", id, msg);
    } else {
        Log::gpu("[{}] {}", id, msg);
    }
}

void enableDebugOutput() {
    GLint flags = 0;
    glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
    if ((flags & GL_CONTEXT_FLAG_DEBUG_BIT) == 0) {
        return; // release build, or the driver refused a debug context
    }
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS); // callback fires on the offending call's stack
    glDebugMessageCallback(glDebugCallback, nullptr);
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, nullptr,
                          GL_FALSE);
}

} // namespace

auto Application::create() -> std::expected<Application, std::string> {
    auto window = Window::create(Application::WIDTH, Application::HEIGHT, Application::TITLE);
    if (!window) {
        return std::unexpected{std::move(window.error())};
    }
    glfwMakeContextCurrent(window->handle());

    if (gladLoadGL(reinterpret_cast<GLADloadfunc>(glfwGetProcAddress)) == 0) {
        return std::unexpected{"gladLoadGL() failed"};
    }
    enableDebugOutput();

    int fbw = 0;
    int fbh = 0;
    glfwGetFramebufferSize(window->handle(), &fbw, &fbh);
    glViewport(0, 0, fbw, fbh);

    Application app{std::move(*window)};

    // default shader; call app.loadShader(...) for more.
    if (auto r = app.loadShader("triangle", "triangle.vert", "triangle.frag"); !r) {
        return std::unexpected{std::move(r.error())};
    }

    if (auto r = app.loadShader("triangle2", "triangle.vert", "triangle2.frag"); !r) {
        return std::unexpected{std::move(r.error())};
    }

    return app;
}

auto Application::loadShader(std::string name, std::string_view vert, std::string_view frag)
    -> std::expected<void, std::string> {
    auto sh = Shader::create(vert, frag);
    if (!sh) {
        return std::unexpected{std::move(sh.error())};
    }
    shaders_.insert_or_assign(std::move(name), std::move(*sh));
    return {};
}

auto Application::shader(std::string_view name) -> Shader& {
    auto it = shaders_.find(name);
    assert(it != shaders_.end() && "shader not loaded");
    return it->second;
}

void Application::processInput() {
    if (window.isKeyPressed(GLFW_KEY_ESCAPE)) {
        window.requestClose();
    }

    // rising edge only, else a held key toggles every frame
    const bool down = window.isKeyPressed(GLFW_KEY_GRAVE_ACCENT);
    if (down && !debugKeyDown_) {
        debugMode_ = !debugMode_;
        glPolygonMode(GL_FRONT_AND_BACK, debugMode_ ? GL_LINE : GL_FILL);
    }
    debugKeyDown_ = down;
}

void Application::pollAndSwap() {
    glfwPollEvents();
    window.swapBuffers();
}
