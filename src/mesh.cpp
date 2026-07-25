#include "mesh.hpp"

#include <glad/gl.h>
#include <span>
#include <utility>

auto Mesh::create(std::span<const float> vertices, std::span<const GLuint> indices,
                  GLsizei vertexStride, std::span<const Mesh::Attrib> attribs) -> Mesh {
    const GLsizeiptr vBytes = static_cast<GLsizeiptr>(vertices.size_bytes());
    const GLsizeiptr iBytes = static_cast<GLsizeiptr>(indices.size_bytes());

    Mesh mesh;
    mesh.vertexCount_ = static_cast<GLsizei>(vBytes / vertexStride);
    mesh.indexCount_ = static_cast<GLsizei>(indices.size());
    mesh.indexOffset_ = vBytes; // indices packed right after vertices

    glCreateVertexArrays(1, &mesh.vao_);
    glCreateBuffers(1, &mesh.vbo_);

    // DYNAMIC_STORAGE_BIT: allocation is immutable, contents are not — the two
    // SubData uploads below need it.
    glNamedBufferStorage(mesh.vbo_, vBytes + iBytes, nullptr, GL_DYNAMIC_STORAGE_BIT);
    glNamedBufferSubData(mesh.vbo_, 0, vBytes, vertices.data());
    if (iBytes > 0) {
        glNamedBufferSubData(mesh.vbo_, vBytes, iBytes, indices.data());
    }

    for (const auto& a : attribs) {
        glEnableVertexArrayAttrib(mesh.vao_, a.location);
        glVertexArrayAttribFormat(mesh.vao_, a.location, a.components, a.type, a.normalize,
                                  a.offset);
        glVertexArrayAttribBinding(mesh.vao_, a.location, 0);
    }

    glVertexArrayVertexBuffer(mesh.vao_, 0, mesh.vbo_, 0, vertexStride);
    if (mesh.indexCount_ > 0) {
        glVertexArrayElementBuffer(mesh.vao_, mesh.vbo_);
    }

    return mesh;
}

void Mesh::draw(GLenum mode) const {
    glBindVertexArray(vao_);
    if (indexCount_ > 0) {
        glDrawElements(mode, indexCount_, GL_UNSIGNED_INT, reinterpret_cast<void*>(indexOffset_));
    } else {
        glDrawArrays(mode, 0, vertexCount_);
    }
}

Mesh::~Mesh() {
    glDeleteVertexArrays(1, &vao_); // 0 is silently ignored by GL
    glDeleteBuffers(1, &vbo_);
}

Mesh::Mesh(Mesh&& o) noexcept
    : vao_{std::exchange(o.vao_, 0)}, vbo_{std::exchange(o.vbo_, 0)},
      vertexCount_{std::exchange(o.vertexCount_, 0)}, indexCount_{std::exchange(o.indexCount_, 0)},
      indexOffset_{std::exchange(o.indexOffset_, 0)} {
}

auto Mesh::operator=(Mesh&& o) noexcept -> Mesh& {
    std::swap(vao_, o.vao_);
    std::swap(vbo_, o.vbo_);
    std::swap(vertexCount_, o.vertexCount_);
    std::swap(indexCount_, o.indexCount_);
    std::swap(indexOffset_, o.indexOffset_);
    return *this;
}
