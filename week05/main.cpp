// Неделя 5. Индексный буфер (EBO) и glDrawElements.
//
// Сцены (клавиши 1–5):
//   1 — квадрат: 4 вершины + 6 индексов, летает по кругу
//   2 — тот же VBO, но индексы {0, 1, 2}: остаётся один треугольник
//   3 — правильный пятиугольник: 5 вершин, 3 треугольника, 9 индексов
//   4 — два квадрата из одного VAO: два раза «uniform + glDrawElements»
//   5 — два квадрата летают по кругу навстречу друг другу
//
// Другие клавиши:
//   P   — следующий примитив: TRIANGLES -> LINE_LOOP -> LINE_STRIP -> POINTS
//   Tab — каркасный режим вкл/выкл
//   Esc — выход

#include "indexed_mesh.hpp"
#include "shader_program.hpp"

#include <GLFW/glfw3.h>

#include <cmath>
#include <cstdio>
#include <string>
#include <vector>

namespace {

const std::string kShaderDir = WEEK05_SHADER_DIR;

constexpr float kPi = 3.14159265358979f;
constexpr float kOrbitRadius = 0.45f;
constexpr float kOrbitSpeed = 1.2f;  // радиан в секунду

enum class Scene { Quad = 1, SingleTriangle, Pentagon, TwoQuads, TwoQuadsOrbit };

const char* sceneName(Scene scene) {
    switch (scene) {
    case Scene::Quad:           return "quad: 4 vertices, 6 indices";
    case Scene::SingleTriangle: return "indices {0,1,2}, count 3";
    case Scene::Pentagon:       return "pentagon: 5 vertices, 9 indices";
    case Scene::TwoQuads:       return "two quads, one VAO";
    case Scene::TwoQuadsOrbit:  return "two quads, opposite orbits";
    }
    return "";
}

struct PrimitiveInfo {
    GLenum type;
    const char* name;
};

constexpr PrimitiveInfo kPrimitives[] = {
    {GL_TRIANGLES, "GL_TRIANGLES"},
    {GL_LINE_LOOP, "GL_LINE_LOOP"},
    {GL_LINE_STRIP, "GL_LINE_STRIP"},
    {GL_POINTS, "GL_POINTS"},
};
constexpr int kPrimitiveCount = sizeof(kPrimitives) / sizeof(kPrimitives[0]);

struct AppState {
    Scene scene = Scene::Quad;
    int primitive = 0;
    bool showEdges = false;
    bool titleDirty = true;
};

// Квадрат: вершины по часовой стрелке, начиная с левой верхней.
//   0 ---- 1
//   |  \   |
//   |   \  |
//   3 ---- 2
// Диагональ 0–2 общая для обоих треугольников.
const std::vector<ColoredVertex> kQuadVertices = {
    {{-0.25f,  0.25f}, {0.95f, 0.30f, 0.30f}},
    {{ 0.25f,  0.25f}, {0.95f, 0.85f, 0.25f}},
    {{ 0.25f, -0.25f}, {0.25f, 0.80f, 0.45f}},
    {{-0.25f, -0.25f}, {0.30f, 0.50f, 0.95f}},
};
const std::vector<GLuint> kQuadIndices = {0, 1, 2,   0, 2, 3};
const std::vector<GLuint> kFirstTriangleOnly = {0, 1, 2};

// Цвет по «оттенку» 0..1 (круг HSV при полной яркости и насыщенности).
void hueToRgb(float hue, float rgb[3]) {
    for (int i = 0; i < 3; ++i) {
        const float k = std::fmod(hue * 6.0f + 4.0f - 2.0f * i, 6.0f);  // сдвиги для R, G, B
        const float x = std::fmin(k, 4.0f - k);
        rgb[i] = std::fmax(0.0f, std::fmin(1.0f, x));
    }
}

// Правильный многоугольник с вершиной сверху. Триангуляция «веером» из
// вершины 0: треугольники (0,1,2), (0,2,3), ... — всего sides-2 штук.
void buildRegularPolygon(int sides, float radius,
                         std::vector<ColoredVertex>& vertices, std::vector<GLuint>& indices) {
    vertices.clear();
    indices.clear();
    for (int i = 0; i < sides; ++i) {
        const float angle = kPi / 2.0f + 2.0f * kPi * i / sides;
        ColoredVertex v{{radius * std::cos(angle), radius * std::sin(angle)}, {}};
        hueToRgb(static_cast<float>(i) / sides, v.color);
        vertices.push_back(v);
    }
    for (int i = 1; i + 1 < sides; ++i) {
        indices.insert(indices.end(), {0u, static_cast<GLuint>(i), static_cast<GLuint>(i + 1)});
    }
}

void onGlfwError(int code, const char* description) {
    std::fprintf(stderr, "GLFW error %d: %s\n", code, description);
}

void onFramebufferResize(GLFWwindow*, int width, int height) {
    glViewport(0, 0, width, height);
}

void onKey(GLFWwindow* window, int key, int, int action, int) {
    if (action != GLFW_PRESS) {
        return;
    }
    auto* state = static_cast<AppState*>(glfwGetWindowUserPointer(window));
    if (key >= GLFW_KEY_1 && key <= GLFW_KEY_5) {
        state->scene = static_cast<Scene>(key - GLFW_KEY_1 + 1);
        std::printf("Scene %d: %s\n", key - GLFW_KEY_1 + 1, sceneName(state->scene));
        state->titleDirty = true;
        return;
    }
    switch (key) {
    case GLFW_KEY_ESCAPE:
        glfwSetWindowShouldClose(window, GLFW_TRUE);
        break;
    case GLFW_KEY_P:
        state->primitive = (state->primitive + 1) % kPrimitiveCount;
        std::printf("Primitive: %s\n", kPrimitives[state->primitive].name);
        state->titleDirty = true;
        break;
    case GLFW_KEY_TAB:
        state->showEdges = !state->showEdges;
        std::printf("Polygon mode: %s\n", state->showEdges ? "GL_LINE" : "GL_FILL");
        break;
    default:
        break;
    }
}

GLFWwindow* openWindow() {
    glfwSetErrorCallback(onGlfwError);
    if (!glfwInit()) {
        return nullptr;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
#endif
    GLFWwindow* window = glfwCreateWindow(1280, 720, "CG Week 05", nullptr, nullptr);
    if (window == nullptr) {
        glfwTerminate();
        return nullptr;
    }
    glfwMakeContextCurrent(window);
    if (gladLoadGL(glfwGetProcAddress) == 0) {
        std::fprintf(stderr, "Failed to load OpenGL functions (GLAD)\n");
        glfwDestroyWindow(window);
        glfwTerminate();
        return nullptr;
    }
    glfwSetFramebufferSizeCallback(window, onFramebufferResize);
    glfwSwapInterval(1);
    return window;
}

int run(GLFWwindow* window) {
    AppState state;
    glfwSetWindowUserPointer(window, &state);
    glfwSetKeyCallback(window, onKey);

    ShaderProgram shader;
    if (!shader.load(kShaderDir + "shape.vert", kShaderDir + "shape.frag")) {
        return 1;
    }
    const GLint shiftLocation = shader.uniformLocation("uShift");
    const GLint aspectLocation = shader.uniformLocation("uAspectFix");

    IndexedMesh quad = createIndexedMesh(kQuadVertices, kQuadIndices);
    IndexedMesh firstTriangle = createIndexView(quad, kFirstTriangleOnly);

    std::vector<ColoredVertex> pentagonVertices;
    std::vector<GLuint> pentagonIndices;
    buildRegularPolygon(5, 0.3f, pentagonVertices, pentagonIndices);
    IndexedMesh pentagon = createIndexedMesh(pentagonVertices, pentagonIndices);

    glPointSize(8.0f);  // чтобы GL_POINTS было видно

    std::printf("Controls: 1-5 scene, P primitive, Tab wireframe, Esc quit\n");
    std::printf("Scene 1: %s\n", sceneName(state.scene));

    double previousTime = glfwGetTime();
    float orbitAngle = 0.0f;

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        // Обновление: угол растёт со скоростью kOrbitSpeed рад/с,
        // независимо от FPS — поэтому умножаем на время кадра dt.
        const double now = glfwGetTime();
        const float dt = static_cast<float>(now - previousTime);
        previousTime = now;
        orbitAngle += kOrbitSpeed * dt;
        const float orbitX = kOrbitRadius * std::cos(orbitAngle);
        const float orbitY = kOrbitRadius * std::sin(orbitAngle);

        if (state.titleDirty) {
            char title[160];
            std::snprintf(title, sizeof(title), "CG Week 05 | %d: %s | %s",
                          static_cast<int>(state.scene), sceneName(state.scene),
                          kPrimitives[state.primitive].name);
            glfwSetWindowTitle(window, title);
            state.titleDirty = false;
        }

        int width = 0;
        int height = 0;
        glfwGetFramebufferSize(window, &width, &height);
        if (width == 0 || height == 0) {
            continue;
        }

        glClearColor(0.09f, 0.10f, 0.13f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glPolygonMode(GL_FRONT_AND_BACK, state.showEdges ? GL_LINE : GL_FILL);

        shader.use();
        glUniform1f(aspectLocation, static_cast<float>(height) / static_cast<float>(width));
        const GLenum primitive = kPrimitives[state.primitive].type;

        switch (state.scene) {
        case Scene::Quad:
            glUniform2f(shiftLocation, orbitX, orbitY);
            drawMesh(quad, primitive);
            break;
        case Scene::SingleTriangle:
            glUniform2f(shiftLocation, orbitX, orbitY);
            drawMesh(firstTriangle, primitive);
            break;
        case Scene::Pentagon:
            glUniform2f(shiftLocation, orbitX, orbitY);
            drawMesh(pentagon, primitive);
            break;
        case Scene::TwoQuads:
            // Один и тот же VAO рисуется дважды — меняется только сдвиг.
            glUniform2f(shiftLocation, -0.5f, 0.0f);
            drawMesh(quad, primitive);
            glUniform2f(shiftLocation, 0.5f, 0.0f);
            drawMesh(quad, primitive);
            break;
        case Scene::TwoQuadsOrbit:
            // Второй квадрат в диаметрально противоположной точке круга.
            glUniform2f(shiftLocation, orbitX, orbitY);
            drawMesh(quad, primitive);
            glUniform2f(shiftLocation, -orbitX, -orbitY);
            drawMesh(quad, primitive);
            break;
        }

        glfwSwapBuffers(window);
    }

    destroyMesh(firstTriangle);  // сначала «вид», потом владелец общего VBO
    destroyMesh(quad);
    destroyMesh(pentagon);
    return 0;
}

} // namespace

int main() {
    GLFWwindow* window = openWindow();
    if (window == nullptr) {
        return 1;
    }
    const int code = run(window);
    glfwDestroyWindow(window);
    glfwTerminate();
    return code;
}
