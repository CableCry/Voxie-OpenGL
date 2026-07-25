#pragma once

#include <glad/gl.h>
#include <span>

// One drawable: vertices + indices packed into a single immutable VBO (megabuffer),
// with a VAO describing the interleaved vertex layout. Owns both, deletes on destruction.
class Mesh {
  public:
    // Describes one vertex attribute in the interleaved vertex data.
    struct Attrib {
        GLuint location;  // shader `layout(location = N)`
        GLint components; // e.g. 3 for a vec3
        GLuint offset;    // byte offset of this attribute within a vertex
        GLenum type = GL_FLOAT;
        GLboolean normalize = GL_FALSE;
    };

    // vertexStride: bytes per vertex (e.g. 3 * sizeof(float) for tight vec3 positions).
    // Empty indices => non-indexed mesh, drawn with glDrawArrays.
    [[nodiscard]] static auto create(std::span<const float> vertices,
                                     std::span<const GLuint> indices, GLsizei vertexStride,
                                     std::span<const Attrib> attribs) -> Mesh;

    // Non-indexed: vertices are drawn in order.
    [[nodiscard]] static auto create(std::span<const float> vertices, GLsizei vertexStride,
                                     std::span<const Attrib> attribs) -> Mesh {
        return create(vertices, {}, vertexStride, attribs);
    }

    void draw(GLenum mode = GL_TRIANGLES) const;

    ~Mesh();
    Mesh(Mesh&& o) noexcept;
    auto operator=(Mesh&& o) noexcept -> Mesh&;
    Mesh(const Mesh&) = delete;
    auto operator=(const Mesh&) -> Mesh& = delete;

  private:
    Mesh() = default;

    GLuint vao_ = 0;
    GLuint vbo_ = 0;
    GLsizei vertexCount_ = 0;
    GLsizei indexCount_ = 0; // 0 => non-indexed
    GLintptr indexOffset_ = 0; // byte offset where indices begin inside vbo_
};
