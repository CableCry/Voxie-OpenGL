#pragma once

#include <expected>
#include <glad/gl.h>
#include <string>
#include <string_view>

#ifndef ASSET_DIR
#error "ASSET_DIR must be defined by the build (see CMakeLists.txt)"
#endif

// One 2D texture loaded from an image file. Owns the GL object, deletes on destruction.
class Texture {
  public:
    // How the texture is sampled. Defaults are the common case: repeat + linear, no mipmaps.
    struct Params {
        GLenum wrapS = GL_REPEAT;
        GLenum wrapT = GL_REPEAT;
        // 0 => LINEAR, or LINEAR_MIPMAP_LINEAR when mipmaps is set. Setting a mipmap filter
        // without mipmaps gives an incomplete (black) texture, so let it follow by default.
        GLenum minFilter = 0;
        GLenum magFilter = GL_LINEAR; // no mipmap variants exist for magnification
        bool mipmaps = false;
        bool flipVertically = true; // images are top-down, GL's texture origin is bottom-left
    };

    // `file` is relative to ASSET_DIR.
    [[nodiscard]] static auto create(std::string_view file, const Params& params)
        -> std::expected<Texture, std::string>;

    // Overload, not a `= {}` default: Params' member initializers aren't parsed yet at
    // that point in the class. Same reason Application::loadTexture has two overloads.
    [[nodiscard]] static auto create(std::string_view file) -> std::expected<Texture, std::string> {
        return create(file, Params{});
    }

    // Binds to a texture unit; the sampler uniform holds the same unit number.
    void bind(GLuint unit) const;

    [[nodiscard]] auto id() const noexcept -> GLuint {
        return tex_;
    }
    [[nodiscard]] auto width() const noexcept -> int {
        return width_;
    }
    [[nodiscard]] auto height() const noexcept -> int {
        return height_;
    }

    ~Texture();
    Texture(Texture&& o) noexcept;
    auto operator=(Texture&& o) noexcept -> Texture&;
    Texture(const Texture&) = delete;
    auto operator=(const Texture&) -> Texture& = delete;

  private:
    Texture() = default;

    GLuint tex_ = 0;
    int width_ = 0;
    int height_ = 0;
};
