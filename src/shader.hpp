#pragma once
#include "logging.hpp"

#include <cstddef>
#include <expected>
#include <filesystem>
#include <format>
#include <fstream>
#include <functional>
#include <glad/gl.h>
#include <sstream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>

#ifndef SHADER_DIR
#error "SHADER_DIR must be defined by the build (see CMakeLists.txt)"
#endif

class Shader {
  public:
    [[nodiscard]] static auto create(std::string_view vertName, std::string_view fragName)
        -> std::expected<Shader, std::string> {
        // SHADER_DIR is absolute, from the build. A relative path would silently
        // depend on the CWD. Built here, not as a static: a std::filesystem::path
        // with static storage can throw where nothing can catch it.
        const std::filesystem::path root{SHADER_DIR};
        const auto vp = root / vertName;
        const auto fp = root / fragName;

        auto vsrc = readFile(vp);
        if (!vsrc) {
            return std::unexpected{std::move(vsrc.error())};
        }
        auto fsrc = readFile(fp);
        if (!fsrc) {
            return std::unexpected{std::move(fsrc.error())};
        }

        auto vs = compile(GL_VERTEX_SHADER, *vsrc, vp);
        if (!vs) {
            return std::unexpected{std::move(vs.error())};
        }
        auto fs = compile(GL_FRAGMENT_SHADER, *fsrc, fp);
        if (!fs) {
            glDeleteShader(*vs);
            return std::unexpected{std::move(fs.error())};
        }

        GLuint prog = glCreateProgram();
        glAttachShader(prog, *vs);
        glAttachShader(prog, *fs);
        glLinkProgram(prog);
        glDeleteShader(*vs); // flagged now, freed when detached at link
        glDeleteShader(*fs);

        GLint ok = GL_FALSE;
        glGetProgramiv(prog, GL_LINK_STATUS, &ok);
        if (ok == GL_FALSE) {
            auto log = infoLog(prog, glGetProgramiv, glGetProgramInfoLog);
            glDeleteProgram(prog);
            return std::unexpected{std::format("link {} + {}:\n{}", vp.string(), fp.string(), log)};
        }
        return Shader{prog};
    }

    ~Shader() {
        glDeleteProgram(program_);
    } // no-op when 0

    Shader(Shader&& o) noexcept
        : program_{std::exchange(o.program_, 0)}, cache_{std::move(o.cache_)} {}
    auto operator=(Shader&& o) noexcept -> Shader& {
        std::swap(program_, o.program_);
        std::swap(cache_, o.cache_);
        return *this;
    }
    Shader(const Shader&) = delete;
    auto operator=(const Shader&) -> Shader& = delete;

    void use() const noexcept {
        glUseProgram(program_);
    }
    [[nodiscard]] auto id() const noexcept -> GLuint {
        return program_;
    }

    void set(std::string_view n, bool v) {
        glUniform1i(loc(n), static_cast<GLint>(v));
    }
    void set(std::string_view n, int v) {
        glUniform1i(loc(n), v);
    }
    void set(std::string_view n, float v) {
        glUniform1f(loc(n), v);
    }
    void set(std::string_view n, float x, float y, float z) {
        glUniform3f(loc(n), x, y, z);
    }
    void set(std::string_view n, float x, float y, float z, float w) {
        glUniform4f(loc(n), x, y, z, w);
    }

  private:
    explicit Shader(GLuint p) noexcept : program_{p} {}

    struct StringHash {
        using is_transparent = void;
        auto operator()(std::string_view s) const noexcept -> std::size_t {
            return std::hash<std::string_view>{}(s);
        }
    };

    auto loc(std::string_view name) -> GLint {
        if (auto it = cache_.find(name); it != cache_.end()) {
            return it->second;
        }
        std::string key{name}; // null-terminated copy for the C API
        const GLint l = glGetUniformLocation(program_, key.c_str());
        if (l < 0) {
            Log::warn("uniform '{}' not found (program {})", name, program_);
        }
        return cache_.emplace(std::move(key), l).first->second;
    }

    static auto readFile(const std::filesystem::path& p)
        -> std::expected<std::string, std::string> {
        std::ifstream in{p, std::ios::binary};
        if (!in) {
            return std::unexpected{std::format("cannot open {}", p.string())};
        }
        std::ostringstream ss;
        ss << in.rdbuf();
        if (in.bad()) {
            return std::unexpected{std::format("read failed: {}", p.string())};
        }
        return std::move(ss).str();
    }

    template <typename GetIv, typename GetLog>
    static auto infoLog(GLuint obj, GetIv getiv, GetLog getlog) -> std::string {
        GLint len = 0;
        getiv(obj, GL_INFO_LOG_LENGTH, &len);
        if (len <= 1) { // len includes the terminator GL writes
            return "(no info log)";
        }
        std::string log(static_cast<std::size_t>(len), '\0');
        GLsizei written = 0;
        getlog(obj, len, &written, log.data());
        log.resize(static_cast<std::size_t>(written)); // drop GL's trailing NUL
        return log;
    }

    static auto compile(GLenum stage, const std::string& src, const std::filesystem::path& p)
        -> std::expected<GLuint, std::string> {
        const GLuint sh = glCreateShader(stage);
        const char* data = src.c_str();
        glShaderSource(sh, 1, &data, nullptr);
        glCompileShader(sh);

        GLint ok = GL_FALSE;
        glGetShaderiv(sh, GL_COMPILE_STATUS, &ok);
        if (ok == GL_FALSE) {
            auto log = infoLog(sh, glGetShaderiv, glGetShaderInfoLog);
            glDeleteShader(sh);
            return std::unexpected{std::format("compile {}:\n{}", p.string(), log)};
        }
        return sh;
    }

    GLuint program_ = 0;
    std::unordered_map<std::string, GLint, StringHash, std::equal_to<>> cache_;
};
