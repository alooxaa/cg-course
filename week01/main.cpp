// Неделя 1. Окно OpenGL 3.3 Core: анимированный фон, переключение фона
// пробелом, счётчик FPS.
//
// Управление:
//   Space — анимированный фон <-> белый фон
//   V     — включить/выключить вертикальную синхронизацию (VSync)
//   Esc   — выход

#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include <cmath>
#include <cstdio>

namespace {

constexpr int kWindowWidth = 1280;
constexpr int kWindowHeight = 720;
constexpr const char* kWindowTitle = "CG Week 01";

struct Color {
    float r, g, b;
};

enum class Background { Animated, White };

// Всё, что меняется от нажатий клавиш, хранится в одном месте.
struct AppState {
    Background background = Background::Animated;
    bool vsync = false;
};

// Считает кадры и раз в секунду сообщает средний FPS.
class FpsCounter {
public:
    explicit FpsCounter(double startTime) : periodStart_(startTime) {}

    // Вызывается один раз за кадр. Возвращает true, когда прошла секунда
    // и в fps записано новое значение.
    bool frame(double now, double& fps) {
        ++frames_;
        const double elapsed = now - periodStart_;
        if (elapsed < 1.0) {
            return false;
        }
        fps = frames_ / elapsed;
        frames_ = 0;
        periodStart_ = now;
        return true;
    }

private:
    double periodStart_;
    int frames_ = 0;
};

void onGlfwError(int code, const char* description) {
    std::fprintf(stderr, "GLFW error %d: %s\n", code, description);
}

// Размер окна изменился — растягиваем область рисования на всё окно.
void onFramebufferResize(GLFWwindow*, int width, int height) {
    glViewport(0, 0, width, height);
}

// GLFW вызывает эту функцию из glfwPollEvents() при каждом событии клавиатуры.
// action == GLFW_PRESS приходит ровно один раз на нажатие (удержание даёт
// GLFW_REPEAT), поэтому отдельно помнить «была ли клавиша нажата» не нужно.
void onKey(GLFWwindow* window, int key, int /*scancode*/, int action, int /*mods*/) {
    if (action != GLFW_PRESS) {
        return;
    }
    auto* state = static_cast<AppState*>(glfwGetWindowUserPointer(window));

    switch (key) {
    case GLFW_KEY_ESCAPE:
        glfwSetWindowShouldClose(window, GLFW_TRUE);
        break;
    case GLFW_KEY_SPACE:
        state->background = state->background == Background::Animated ? Background::White : Background::Animated;
        break;
    case GLFW_KEY_V:
        state->vsync = !state->vsync;
        glfwSwapInterval(state->vsync ? 1 : 0);
        std::printf("VSync: %s\n", state->vsync ? "on" : "off");
        break;
    default:
        break;
    }
}

// Три синусоиды, сдвинутые друг относительно друга на треть периода,
// дают плавный перелив цветов. Значения держим в диапазоне 0.1..0.5,
// чтобы фон оставался тёмным и не резал глаза.
Color animatedBackground(double time) {
    constexpr double kThird = 2.0 * 3.14159265358979 / 3.0;
    auto wave = [time](double shift) {
        return static_cast<float>(0.1 + 0.4 * (0.5 + 0.5 * std::sin(0.7 * time + shift)));
    };
    return {wave(0.0), wave(kThird), wave(2.0 * kThird)};
}

void printGlInfo() {
    auto str = [](GLenum name) { return reinterpret_cast<const char*>(glGetString(name)); };
    std::printf("Vendor:   %s\n", str(GL_VENDOR));
    std::printf("Renderer: %s\n", str(GL_RENDERER));
    std::printf("OpenGL:   %s\n", str(GL_VERSION));
    std::printf("GLSL:     %s\n", str(GL_SHADING_LANGUAGE_VERSION));
}

GLFWwindow* createWindow() {
    // Просим именно OpenGL 3.3 Core: без устаревших функций старого OpenGL.
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
#endif

    GLFWwindow* window = glfwCreateWindow(kWindowWidth, kWindowHeight, kWindowTitle, nullptr, nullptr);
    if (window == nullptr) {
        return nullptr;
    }

    // Контекст OpenGL привязывается к потоку: все вызовы gl* дальше идут в это окно.
    glfwMakeContextCurrent(window);

    // GLAD находит адреса функций OpenGL в драйвере видеокарты.
    // Без этого шага любой вызов gl* упадёт.
    if (gladLoadGL(glfwGetProcAddress) == 0) {
        std::fprintf(stderr, "Failed to load OpenGL functions (GLAD)\n");
        glfwDestroyWindow(window);
        return nullptr;
    }
    return window;
}

} // namespace

int main() {
    glfwSetErrorCallback(onGlfwError);
    if (!glfwInit()) {
        return 1;
    }

    GLFWwindow* window = createWindow();
    if (window == nullptr) {
        glfwTerminate();
        return 1;
    }

    AppState state;
    glfwSetWindowUserPointer(window, &state);
    glfwSetKeyCallback(window, onKey);
    glfwSetFramebufferSizeCallback(window, onFramebufferResize);
    glfwSwapInterval(state.vsync ? 1 : 0);

    printGlInfo();
    std::printf("Controls: Space - background, V - vsync, Esc - quit\n");

    FpsCounter fpsCounter(glfwGetTime());
    char title[128];

    // Главный цикл: обработать события -> нарисовать кадр -> показать его.
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        const double now = glfwGetTime();
        const Color bg = state.background == Background::White ? Color{1.0f, 1.0f, 1.0f} : animatedBackground(now);
        glClearColor(bg.r, bg.g, bg.b, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Рисовали в невидимый задний буфер — меняем его местами с экранным.
        glfwSwapBuffers(window);

        double fps = 0.0;
        if (fpsCounter.frame(now, fps)) {
            std::printf("FPS: %.0f\n", fps);
            std::snprintf(title, sizeof(title), "%s | %.0f FPS", kWindowTitle, fps);
            glfwSetWindowTitle(window, title);
        }
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
