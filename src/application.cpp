#include "application.hpp"

#include "glad/gl.h"
#include "GLFW/glfw3.h"
#include "logging.hpp"
#include "shader.hpp"

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

    auto shader = Shader::create("triangle.vert", "triangle.frag");
    if (!shader) {
        return std::unexpected{std::move(shader.error())};
    }

    return Application{std::move(*window), std::move(*shader)};
}

void Application::processInput() {
    if (window.isKeyPressed(GLFW_KEY_ESCAPE)) {
        window.requestClose();
    }
}

void Application::pollAndSwap() {
    glfwPollEvents();
    window.swapBuffers();
}
