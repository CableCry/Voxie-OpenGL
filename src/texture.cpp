#include "texture.hpp"

#include <bit>
#include <expected>
#include <filesystem>
#include <format>
#include <glad/gl.h>
#include <memory>
#include <stb_image.h>
#include <string>
#include <string_view>
#include <utility>

namespace {

struct StbDeleter {
    void operator()(stbi_uc* p) const noexcept {
        stbi_image_free(p);
    }
};

// Sized internal format + the matching client-side format, by channel count.
auto formatsFor(int channels) -> std::pair<GLenum, GLenum> {
    switch (channels) {
    case 1:
        return {GL_R8, GL_RED};
    case 2:
        return {GL_RG8, GL_RG};
    case 3:
        return {GL_RGB8, GL_RGB};
    default:
        return {GL_RGBA8, GL_RGBA};
    }
}

} // namespace

auto Texture::create(std::string_view file, const Params& params)
    -> std::expected<Texture, std::string> {
    // Absolute, from the build: a relative path would silently depend on the CWD.
    const std::filesystem::path path = std::filesystem::path{ASSET_DIR} / file;

    stbi_set_flip_vertically_on_load(params.flipVertically ? 1 : 0);

    int w = 0;
    int h = 0;
    int channels = 0;
    const std::unique_ptr<stbi_uc, StbDeleter> pixels{
        stbi_load(path.string().c_str(), &w, &h, &channels, 0)};
    if (!pixels) {
        return std::unexpected{std::format("load {}: {}", path.string(), stbi_failure_reason())};
    }

    const auto [internalFormat, format] = formatsFor(channels);
    // Mip chain length: every halving down to 1x1.
    const GLsizei levels =
        params.mipmaps ? static_cast<GLsizei>(std::bit_width(static_cast<unsigned>(w > h ? w : h)))
                       : 1;

    Texture tex;
    tex.width_ = w;
    tex.height_ = h;
    glCreateTextures(GL_TEXTURE_2D, 1, &tex.tex_);
    glTextureStorage2D(tex.tex_, levels, internalFormat, w, h);

    // Rows are tightly packed; GL defaults to 4-byte alignment and would skew any image
    // whose row length isn't a multiple of 4 (e.g. most RGB ones).
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTextureSubImage2D(tex.tex_, 0, 0, 0, w, h, format, GL_UNSIGNED_BYTE, pixels.get());
    if (params.mipmaps) {
        glGenerateTextureMipmap(tex.tex_);
    }

    const GLenum minFilter =
        params.minFilter != 0 ? params.minFilter
                              : (params.mipmaps ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
    glTextureParameteri(tex.tex_, GL_TEXTURE_WRAP_S, static_cast<GLint>(params.wrapS));
    glTextureParameteri(tex.tex_, GL_TEXTURE_WRAP_T, static_cast<GLint>(params.wrapT));
    glTextureParameteri(tex.tex_, GL_TEXTURE_MIN_FILTER, static_cast<GLint>(minFilter));
    glTextureParameteri(tex.tex_, GL_TEXTURE_MAG_FILTER, static_cast<GLint>(params.magFilter));

    return tex;
}

void Texture::bind(GLuint unit) const {
    glBindTextureUnit(unit, tex_);
}

Texture::~Texture() {
    glDeleteTextures(1, &tex_); // 0 is silently ignored by GL
}

Texture::Texture(Texture&& o) noexcept
    : tex_{std::exchange(o.tex_, 0)}, width_{std::exchange(o.width_, 0)},
      height_{std::exchange(o.height_, 0)} {}

auto Texture::operator=(Texture&& o) noexcept -> Texture& {
    std::swap(tex_, o.tex_);
    std::swap(width_, o.width_);
    std::swap(height_, o.height_);
    return *this;
}
