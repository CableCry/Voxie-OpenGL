#include "application.hpp"

#include "GLFW/glfw3.h"

#include <expected>
#include <iostream>
#include <optional>
#include <string>

namespace {
struct Log {
    static auto info() -> std::ostream& {
        return std::cout << "[info ] ";
    }
    static auto ok() -> std::ostream& {
        return std::cout << "[ ok  ] ";
    }
    static auto warn() -> std::ostream& {
        return std::cerr << "[warn ] ";
    }
    static auto error() -> std::ostream& {
        return std::cerr << "[ERROR] ";
    }
    static auto gpu() -> std::ostream& {
        return std::cerr << "[ gpu ] ";
    }
    static void section(std::string_view title) {
        std::cout << "\n--- " << title << " ---\n";
    }
};

auto bytes(uint64_t n) -> std::string {
    char buf[64];
    if (n >= (1ULL << 30))
        std::snprintf(buf, sizeof buf, "%.2f GiB", n / double(1ull << 30));
    else if (n >= (1ull << 20))
        std::snprintf(buf, sizeof buf, "%.2f MiB", n / double(1ull << 20));
    else if (n >= (1ull << 10))
        std::snprintf(buf, sizeof buf, "%.2f KiB", n / double(1ull << 10));
    else
        std::snprintf(buf, sizeof buf, "%llu B", (unsigned long long) n);
    return buf;
}

void glfwErrorCallback(int code, const char* description) {
    Log::error() << "GLFW error " << code << ": " << description << "\n";
}

constexpr uint32_t kWidth = 1280;
constexpr uint32_t kHeight = 720;

} // namespace

struct GlfwSession {
    GlfwSession() {
        if (!glfwInit()) {
            throw std::runtime_error("glfwInit failed");
        }
    }
    ~GlfwSession() {
        glfwTerminate();
    }
    GlfwSession(const GlfwSession&) = delete;
    auto operator=(const GlfwSession&) -> GlfwSession& = delete;
};

void GLFWwindowDeleter::operator()(GLFWwindow* w) const noexcept {
    glfwDestroyWindow(w);
}

auto CreateWindow(int32_t w, int32_t h, const std::string& title) -> WindowPtr {
    return WindowPtr{glfwCreateWindow(w, h, title.c_str(), nullptr, nullptr)};
}

auto InitializeWindow(int32_t width, int32_t height, const std::string& title) -> WindowPtr {
    glfwSetErrorCallback(glfwErrorCallback);

    GlfwSession glfw;
    Log::ok() << "GLFW " << glfwGetVersionString() << '\n';

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    auto window = CreateWindow(width, height, title);
    if (!window) {
        Log::error() << "glfwCreateWindow() failed\n";
        glfwTerminate();
        return nullptr;
    }
    return window;
}
