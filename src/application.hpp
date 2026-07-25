#pragma once

#include "shader.hpp"
#include "texture.hpp"

#include <expected>
#include <glad/gl.h> // must precede GLFW: GLFW pulls in a system GL header otherwise
#include <GLFW/glfw3.h>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>

// --- GLFW lifetime ----------------------------------------------------------

// ponytail: one session per Window. Fine while exactly one Window exists; the first
// session destroyed calls glfwTerminate() out from under any others. Hoist to a
// refcounted/shared session if a second window ever appears.
class GlfwSession {
  public:
    [[nodiscard]] static auto create() -> std::expected<GlfwSession, std::string>;

    ~GlfwSession();
    GlfwSession(GlfwSession&& o) noexcept : owns_{std::exchange(o.owns_, false)} {};
    auto operator=(GlfwSession&& o) noexcept -> GlfwSession&;
    GlfwSession(const GlfwSession&) = delete;
    auto operator=(const GlfwSession&) -> GlfwSession& = delete;

    static void glfwErrorCallback(int code, const char* description);

  private:
    GlfwSession() = default;
    bool owns_ = true;
};

// --- Window -----------------------------------------------------------------

class Window {
  public:
    struct GLFWwindowDeleter {
        void operator()(GLFWwindow* w) const noexcept;
    };

    using WindowPtr = std::unique_ptr<GLFWwindow, GLFWwindowDeleter>;

    [[nodiscard]] static auto create(int w, int h, const char* title)
        -> std::expected<Window, std::string>;

    [[nodiscard]] auto handle() noexcept -> GLFWwindow*;
    [[nodiscard]] auto handle() const noexcept -> const GLFWwindow*;

    void swapBuffers() noexcept;
    void requestClose() noexcept;
    [[nodiscard]] auto shouldClose() const noexcept -> bool;

    [[nodiscard]] auto isKeyPressed(int key) const noexcept -> bool;

    // ponytail: polled, not a resize callback. GLFW callbacks only carry a GLFWwindow*,
    // so reaching back to C++ state needs glfwSetWindowUserPointer(this) -- which dangles
    // the moment a move-only Window is moved. Poll once a frame instead; events are
    // already pumped in pollAndSwap(). Switch to a callback if resize must be handled
    // mid-frame or while the OS blocks the loop (live-drag on Win32).
    [[nodiscard]] auto framebufferSize() const noexcept -> std::pair<int, int>;

    // static: acts on the current GL context, not on this window's state.
    static void clearScreen() noexcept;

  private:
    Window(GlfwSession s, WindowPtr w) : session_{std::move(s)}, window_{std::move(w)} {}

    GlfwSession session_; // declared first => destroyed last
    WindowPtr window_;
};

// --- Application ------------------------------------------------------------

class Application {
  public:
    static constexpr int WIDTH = 1920;
    static constexpr int HEIGHT = 1080;
    // const char*, not std::string: a constexpr std::string only compiles while the
    // literal fits in SSO, so a longer title would break the build.
    static constexpr const char* TITLE = "BINDOW";

    [[nodiscard]] static auto create() -> std::expected<Application, std::string>;

    void processInput();
    void pollAndSwap();

    // Polls the framebuffer size and re-syncs glViewport when it changed. Returns the new
    // size (as floats, ready for a `resolution` uniform) only on a change, else nullopt --
    // pushing it to shaders is the caller's job. 0x0 (minimized) is ignored: a 0 resolution
    // NaNs the aspect divide in the shader.
    auto syncViewport() -> std::optional<std::pair<float, float>>;

    // Compile vert+frag and store under `name`, replacing any existing shader of that name.
    [[nodiscard]] auto loadShader(std::string name, std::string_view vert, std::string_view frag)
        -> std::expected<void, std::string>;
    // Access a stored shader. Precondition: `name` was loaded (asserted).
    [[nodiscard]] auto shader(std::string_view name) -> Shader&;

    // Load `file` (relative to ASSET_DIR) and store under `name`, replacing any existing
    // texture of that name.
    [[nodiscard]] auto loadTexture(std::string name, std::string_view file,
                                   const Texture::Params& params)
        -> std::expected<void, std::string>;
    [[nodiscard]] auto loadTexture(std::string name, std::string_view file)
        -> std::expected<void, std::string> {
        return loadTexture(std::move(name), file, Texture::Params{});
    }
    // Access a stored texture. Precondition: `name` was loaded (asserted).
    [[nodiscard]] auto texture(std::string_view name) -> Texture&;

    Window window;

  private:
    explicit Application(Window window) : window{std::move(window)} {};

    // transparent hash: look up by string_view without allocating a key each frame
    struct StringHash {
        using is_transparent = void;
        auto operator()(std::string_view s) const noexcept -> std::size_t {
            return std::hash<std::string_view>{}(s);
        }
    };
    std::unordered_map<std::string, Shader, StringHash, std::equal_to<>> shaders_;
    std::unordered_map<std::string, Texture, StringHash, std::equal_to<>> textures_;

    bool debugMode_ = false;
    bool debugKeyDown_ = false;         // prev-frame key state, for rising-edge toggle
    std::pair<int, int> lastSize_{0, 0}; // {0,0} => first frame always syncs
};
