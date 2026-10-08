#include "indexed_mesh.hpp"

#include <cstddef>

namespace {

// Описывает формат вершины для VAO, который сейчас привязан.
void describeVertexLayout() {
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(ColoredVertex),
                          reinterpret_cast<const void*>(offsetof(ColoredVertex, position)));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(ColoredVertex),
                          reinterpret_cast<const void*>(offsetof(ColoredVertex, color)));
    glEnableVertexAttribArray(1);
}

// EBO запоминается внутри VAO, поэтому создавать его нужно,
// пока VAO привязан. По этой же причине EBO нельзя отвязывать
// (glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0)) до glBindVertexArray(0) —
// иначе VAO «забудет» свои индексы.
GLuint uploadIndices(const std::vector<GLuint>& indices) {
    GLuint ebo = 0;
    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);
    return ebo;
}

} // namespace

IndexedMesh createIndexedMesh(const std::vector<ColoredVertex>& vertices,
                              const std::vector<GLuint>& indices) {
    IndexedMesh mesh;
    mesh.ownsVbo = true;
    mesh.indexCount = static_cast<GLsizei>(indices.size());

    glGenVertexArrays(1, &mesh.vao);
    glBindVertexArray(mesh.vao);

    glGenBuffers(1, &mesh.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(ColoredVertex), vertices.data(), GL_STATIC_DRAW);

    mesh.ebo = uploadIndices(indices);
    describeVertexLayout();

    glBindVertexArray(0);
    return mesh;
}

IndexedMesh createIndexView(const IndexedMesh& source, const std::vector<GLuint>& indices) {
    IndexedMesh view;
    view.vbo = source.vbo;
    view.indexCount = static_cast<GLsizei>(indices.size());

    glGenVertexArrays(1, &view.vao);
    glBindVertexArray(view.vao);
    glBindBuffer(GL_ARRAY_BUFFER, source.vbo);
    view.ebo = uploadIndices(indices);
    describeVertexLayout();
    glBindVertexArray(0);
    return view;
}

void drawMesh(const IndexedMesh& mesh, GLenum primitive) {
    glBindVertexArray(mesh.vao);
    // Второй аргумент — число ИНДЕКСОВ, а не вершин.
    glDrawElements(primitive, mesh.indexCount, GL_UNSIGNED_INT, nullptr);
}

void destroyMesh(IndexedMesh& mesh) {
    glDeleteBuffers(1, &mesh.ebo);
    if (mesh.ownsVbo) {
        glDeleteBuffers(1, &mesh.vbo);
    }
    glDeleteVertexArrays(1, &mesh.vao);
    mesh = {};
}
