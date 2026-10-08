// Меш с индексным буфером (EBO): вершины хранятся один раз,
// а треугольники собираются из них по номерам.
#pragma once

#include <glad/gl.h>

#include <vector>

struct ColoredVertex {
    float position[2];
    float color[3];
};

struct IndexedMesh {
    GLuint vao = 0;
    GLuint vbo = 0;   // 0, если буфер вершин чужой (см. createIndexView)
    GLuint ebo = 0;
    GLsizei indexCount = 0;
    bool ownsVbo = false;
};

// Создаёт VAO + собственный VBO с вершинами + EBO с индексами.
IndexedMesh createIndexedMesh(const std::vector<ColoredVertex>& vertices,
                              const std::vector<GLuint>& indices);

// Создаёт новый VAO и EBO поверх УЖЕ существующего VBO другого меша.
// Вершины не копируются — меняется только то, какие из них соединять.
IndexedMesh createIndexView(const IndexedMesh& source, const std::vector<GLuint>& indices);

// glDrawElements по всем индексам меша выбранным примитивом.
void drawMesh(const IndexedMesh& mesh, GLenum primitive);

void destroyMesh(IndexedMesh& mesh);
