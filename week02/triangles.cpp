// Неделя 2, часть 1. Два треугольника из одного VBO, цвет задаётся uniform-ом.
//
// Управление:
//   L   — примитив GL_TRIANGLES <-> GL_LINE_LOOP (только контур)
//   W   — каркасный режим glPolygonMode(GL_LINE) вкл/выкл
//   C   — следующая палитра цветов
//   Esc — выход

#include "gl_helpers.hpp"

#include <cstddef>
#include <cstdio>

namespace {

struct Vertex {
    float x, y, z;
};

struct Color {
    float r, g, b;
};

// Координаты сразу в NDC (нормализованных координатах устройства):
// и x, и y видимой области лежат в [-1, 1], центр окна — (0, 0).
// Левый треугольник смотрит вершиной вверх, правый — вниз.
constexpr Vertex kVertices[] = {
    {-0.85f, -0.55f, 0.0f}, {-0.15f, -0.55f, 0.0f}, {-0.50f,  0.55f, 0.0f},
    { 0.15f,  0.55f, 0.0f}, { 0.85f,  0.55f, 0.0f}, { 0.50f, -0.55f, 0.0f},
};
constexpr int kVerticesPerTriangle = 3;
constexpr int kTriangleCount = 2;

// Каждая палитра — по одному цвету на треугольник.
constexpr Color kPalettes[][kTriangleCount] = {
    {{0.98f, 0.62f, 0.15f}, {0.20f, 0.75f, 0.70f}},  // оранжевый + бирюзовый
    {{0.90f, 0.25f, 0.45f}, {0.45f, 0.40f, 0.95f}},  // малиновый + фиолетовый
    {{0.95f, 0.90f, 0.30f}, {0.35f, 0.85f, 0.35f}},  // жёлтый + зелёный
};
constexpr int kPaletteCount = sizeof(kPalettes) / sizeof(kPalettes[0]);

const char* const kVertexShader = R"(#version 330 core
layout (location = 0) in vec3 inPosition;
void main() {
    gl_Position = vec4(inPosition, 1.0);
}
)";

const char* const kFragmentShader = R"(#version 330 core
uniform vec3 uFillColor;
out vec4 fragColor;
void main() {
    fragColor = vec4(uFillColor, 1.0);
}
)";

struct AppState {
    bool outlineOnly = false;  // L
    bool showEdges = false;    // W
    int palette = 0;           // C
};

void onKey(GLFWwindow* window, int key, int, int action, int) {
    if (action != GLFW_PRESS) {
        return;
    }
    auto* state = static_cast<AppState*>(glfwGetWindowUserPointer(window));
    switch (key) {
    case GLFW_KEY_ESCAPE:
        glfwSetWindowShouldClose(window, GLFW_TRUE);
        break;
    case GLFW_KEY_L:
        state->outlineOnly = !state->outlineOnly;
        std::printf("Primitive: %s\n", state->outlineOnly ? "GL_LINE_LOOP" : "GL_TRIANGLES");
        break;
    case GLFW_KEY_W:
        state->showEdges = !state->showEdges;
        std::printf("Polygon mode: %s\n", state->showEdges ? "GL_LINE" : "GL_FILL");
        break;
    case GLFW_KEY_C:
        state->palette = (state->palette + 1) % kPaletteCount;
        std::printf("Palette: %d\n", state->palette);
        break;
    default:
        break;
    }
}

// VAO запоминает, откуда и в каком формате брать вершины;
// VBO — сам буфер с вершинами в памяти видеокарты.
struct Mesh {
    GLuint vao = 0;
    GLuint vbo = 0;
};

Mesh createMesh(const Vertex* vertices, std::size_t count) {
    Mesh mesh;
    glGenVertexArrays(1, &mesh.vao);
    glGenBuffers(1, &mesh.vbo);

    glBindVertexArray(mesh.vao);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
    glBufferData(GL_ARRAY_BUFFER, count * sizeof(Vertex), vertices, GL_STATIC_DRAW);

    // Атрибут 0 (inPosition): 3 float подряд, между вершинами sizeof(Vertex) байт.
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), nullptr);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
    return mesh;
}

void destroyMesh(Mesh& mesh) {
    glDeleteBuffers(1, &mesh.vbo);
    glDeleteVertexArrays(1, &mesh.vao);
    mesh = {};
}

} // namespace

int main() {
    GLFWwindow* window = openWindow(1280, 720, "CG Week 02 - Triangles");
    if (window == nullptr) {
        return 1;
    }

    AppState state;
    glfwSetWindowUserPointer(window, &state);
    glfwSetKeyCallback(window, onKey);

    GLuint program = buildShaderProgram(kVertexShader, kFragmentShader);
    if (program == 0) {
        glfwTerminate();
        return 1;
    }
    // Ищем uniform один раз: поиск по имени — медленная операция.
    const GLint colorLocation = glGetUniformLocation(program, "uFillColor");

    Mesh mesh = createMesh(kVertices, sizeof(kVertices) / sizeof(kVertices[0]));

    std::printf("Controls: L - line loop, W - wireframe, C - palette, Esc - quit\n");

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        glClearColor(0.08f, 0.09f, 0.12f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glPolygonMode(GL_FRONT_AND_BACK, state.showEdges ? GL_LINE : GL_FILL);
        const GLenum primitive = state.outlineOnly ? GL_LINE_LOOP : GL_TRIANGLES;

        glUseProgram(program);
        glBindVertexArray(mesh.vao);

        // Один буфер, два вызова отрисовки: каждый берёт свои 3 вершины
        // и перед этим получает свой цвет через uniform.
        for (int i = 0; i < kTriangleCount; ++i) {
            const Color& color = kPalettes[state.palette][i];
            glUniform3f(colorLocation, color.r, color.g, color.b);
            glDrawArrays(primitive, i * kVerticesPerTriangle, kVerticesPerTriangle);
        }

        glfwSwapBuffers(window);
    }

    destroyMesh(mesh);
    glDeleteProgram(program);
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
