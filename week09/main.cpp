// Неделя 9: пять вращающихся кубов, view = lookAt, движение с delta time.
#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/type_ptr.hpp>

#include "camera.hpp"

#include <array>
#include <cstddef>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
constexpr int task = WEEK09_TASK;
struct App {
    week09::Camera camera;
    int width = 800, height = 600;
    bool vsync = true;
};

const char* vertexSource = R"(#version 330 core
layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aColor;
uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;
out vec3 color;
void main() {
    gl_Position = uProjection * uView * uModel * vec4(aPosition, 1.0);
    color = aColor;
}
)";
const char* fragmentSource = R"(#version 330 core
in vec3 color;
out vec4 fragmentColor;
void main() { fragmentColor = vec4(color, 1.0); }
)";

GLuint compile(GLenum stage, const char* source) {
    const GLuint shader = glCreateShader(stage);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);
    GLint ok = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log[4096]{};
        glGetShaderInfoLog(shader, sizeof(log), nullptr, log);
        glDeleteShader(shader);
        throw std::runtime_error(log);
    }
    return shader;
}

GLuint makeProgram() {
    const GLuint vertex = compile(GL_VERTEX_SHADER, vertexSource);
    GLuint fragment = 0;
    try { fragment = compile(GL_FRAGMENT_SHADER, fragmentSource); }
    catch (...) { glDeleteShader(vertex); throw; }
    const GLuint program = glCreateProgram();
    glAttachShader(program, vertex);
    glAttachShader(program, fragment);
    glLinkProgram(program);
    glDeleteShader(vertex);
    glDeleteShader(fragment);
    GLint ok = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &ok);
    if (!ok) {
        char log[4096]{};
        glGetProgramInfoLog(program, sizeof(log), nullptr, log);
        glDeleteProgram(program);
        throw std::runtime_error(log);
    }
    return program;
}

struct Vertex { glm::vec3 position, color; };
std::vector<Vertex> cubeVertices() {
    // Нормаль и два ребра каждой грани; cross(u,v) смотрит наружу.
    struct Face { glm::vec3 normal, u, v, color; };
    const Face faces[] = {
        {{0,0,1}, {1,0,0}, {0,1,0}, {1,0,0}},
        {{0,0,-1}, {-1,0,0}, {0,1,0}, {0,1,0}},
        {{-1,0,0}, {0,0,1}, {0,1,0}, {0,0,1}},
        {{1,0,0}, {0,0,-1}, {0,1,0}, {1,1,0}},
        {{0,-1,0}, {1,0,0}, {0,0,1}, {1,0,1}},
        {{0,1,0}, {1,0,0}, {0,0,-1}, {0,1,1}}
    };
    std::vector<Vertex> vertices;
    for (const Face& face : faces) {
        const glm::vec3 centre = face.normal * 0.5f;
        const glm::vec3 u = face.u * 0.5f, v = face.v * 0.5f;
        const glm::vec3 corners[] = {centre-u-v, centre+u-v, centre+u+v, centre-u+v};
        for (int index : {0, 1, 2, 0, 2, 3}) vertices.push_back({corners[index], face.color});
    }
    return vertices;
}

void onKey(GLFWwindow* window, int key, int, int action, int) {
    if (action != GLFW_PRESS) return;
    auto& app = *static_cast<App*>(glfwGetWindowUserPointer(window));
    if (key == GLFW_KEY_ESCAPE) glfwSetWindowShouldClose(window, GLFW_TRUE);
    if (key == GLFW_KEY_R) app.camera.position = {0, 0, 3};
    if (key == GLFW_KEY_O && task == 1) app.camera.orbitTest = !app.camera.orbitTest;
    if (key == GLFW_KEY_U && task == 2) app.camera.worldUp = !app.camera.worldUp;
    if (key == GLFW_KEY_B && task == 3) app.camera.speedBeforeDt = !app.camera.speedBeforeDt;
    if (key == GLFW_KEY_V) { app.vsync = !app.vsync; glfwSwapInterval(app.vsync ? 1 : 0); }
}

week09::Input readInput(GLFWwindow* window) {
    auto held = [window](int key) { return glfwGetKey(window, key) == GLFW_PRESS; };
    return {held(GLFW_KEY_W), held(GLFW_KEY_S), held(GLFW_KEY_A), held(GLFW_KEY_D),
            held(GLFW_KEY_SPACE), held(GLFW_KEY_LEFT_SHIFT), held(GLFW_KEY_LEFT_CONTROL)};
}

void capture(const std::string& path, int width, int height) {
    std::vector<unsigned char> pixels(static_cast<size_t>(width) * height * 3);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadBuffer(GL_BACK);
    glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());
    std::ofstream file(path, std::ios::binary);
    file << "P6\n" << width << ' ' << height << "\n255\n";
    for (int row = height - 1; row >= 0; --row)
        file.write(reinterpret_cast<const char*>(pixels.data() + static_cast<size_t>(row) * width * 3), width * 3);
    if (!file) throw std::runtime_error("Cannot save capture: " + path);
}
}

int main(int argc, char** argv) {
    App app;
    std::string capturePath, demo;
    for (int i = 1; i < argc; ++i) {
        const std::string argument = argv[i];
        if (argument == "--capture" && i + 1 < argc) capturePath = argv[++i];
        else if (argument == "--demo" && i + 1 < argc) demo = argv[++i];
        else if (argument == "--orbit" && task == 1) app.camera.orbitTest = true;
        else {
            std::cerr << "Usage: week09_taskN [--capture image.ppm] [--demo forward|left|up|sprint] [--orbit (task1)]\n";
            return 1;
        }
    }
    if ((!demo.empty() && capturePath.empty()) ||
        (!demo.empty() && demo != "forward" && demo != "left" &&
         !(demo == "up" && task == 2) && !(demo == "sprint" && task == 3))) {
        std::cerr << "Demo requires --capture and a movement supported by this task\n";
        return 1;
    }
    glfwSetErrorCallback([](int, const char* message) { std::cerr << "GLFW: " << message << '\n'; });
    if (!glfwInit()) return 1;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
#endif
    if (!capturePath.empty()) glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    GLFWwindow* window = glfwCreateWindow(app.width, app.height, "Week 09 - Camera", nullptr, nullptr);
    if (!window) { glfwTerminate(); return 1; }
    glfwMakeContextCurrent(window);
    if (!gladLoadGL(glfwGetProcAddress)) { glfwDestroyWindow(window); glfwTerminate(); return 1; }
    glfwSwapInterval(capturePath.empty() ? 1 : 0);
    glfwSetWindowUserPointer(window, &app);
    glfwSetKeyCallback(window, onKey);
    glfwSetFramebufferSizeCallback(window, [](GLFWwindow* w, int width, int height) {
        auto& state = *static_cast<App*>(glfwGetWindowUserPointer(w));
        state.width = width; state.height = height;
        glViewport(0, 0, width, height);
    });
    glfwGetFramebufferSize(window, &app.width, &app.height);
    glViewport(0, 0, app.width, app.height);

    GLuint program = 0, vao = 0, vbo = 0;
    int result = 0;
    try {
        program = makeProgram();
        const auto vertices = cubeVertices();
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, position)));
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, color)));
        glEnableVertexAttribArray(1);
        const GLint modelLocation = glGetUniformLocation(program, "uModel");
        const GLint viewLocation = glGetUniformLocation(program, "uView");
        const GLint projectionLocation = glGetUniformLocation(program, "uProjection");
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glFrontFace(GL_CCW);
        const std::array<glm::vec3, 5> positions = {{
            {0,0,0}, {2,0.5f,-2}, {-1.8f,-0.3f,-1}, {1.2f,-0.8f,-3.5f}, {-2.2f,1,-4}
        }};
        std::cout << "OpenGL: " << glGetString(GL_VERSION) << "\nRenderer: " << glGetString(GL_RENDERER)
                  << "\nTask " << task << ": WASD, R reset, V vsync, Esc exit\n";
        if (task == 1) std::cout << "O: fixed direction / look at origin\n";
        if (task == 2) std::cout << "Space/Left Shift: up/down; U: world Y / camUp\n";
        if (task == 3) std::cout << "Left Ctrl: sprint x2; B: multiply by 2 before/after dt\n";
        double lastFrame = glfwGetTime(), lastTitle = lastFrame;
        int frames = 0, titleFrames = 0;
        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();
            const double now = glfwGetTime();
            const float dt = capturePath.empty() ? static_cast<float>(now-lastFrame) : 1.0f/60.0f;
            lastFrame = now;
            // При сворачивании не делим на ноль и не накапливаем время движения.
            if (app.width == 0 || app.height == 0) { glfwWaitEventsTimeout(0.05); continue; }
            auto input = readInput(window);
            if (!capturePath.empty()) {
                input = {};
                input.forward = demo == "forward" || demo == "sprint";
                input.left = demo == "left";
                input.up = demo == "up";
                input.sprint = demo == "sprint";
            }
            app.camera.move(input, dt, task);
            glClearColor(0.1f, 0.1f, 0.15f, 1);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glUseProgram(program);
            const glm::mat4 view = app.camera.view();
            const glm::mat4 projection = glm::perspective(glm::radians(45.0f),
                static_cast<float>(app.width)/app.height, 0.1f, 100.0f);
            glUniformMatrix4fv(viewLocation, 1, GL_FALSE, glm::value_ptr(view));
            glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, glm::value_ptr(projection));
            const float angle = capturePath.empty() ? static_cast<float>(now) : (frames+1)/60.0f;
            glBindVertexArray(vao);
            for (size_t i = 0; i < positions.size(); ++i) {
                glm::mat4 model = glm::translate(glm::mat4(1), positions[i]);
                model = glm::rotate(model, angle*(0.5f+static_cast<float>(i)*0.3f), glm::vec3(0.3f,1,0));
                glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));
                glDrawArrays(GL_TRIANGLES, 0, 36);
            }
            if (glGetError() != GL_NO_ERROR) throw std::runtime_error("OpenGL rendering error");
            ++frames; ++titleFrames;
            if (!capturePath.empty() && frames == 60) {
                capture(capturePath, app.width, app.height);
                std::cout << "Capture: " << capturePath << " | camPos = (" << app.camera.position.x
                          << ", " << app.camera.position.y << ", " << app.camera.position.z << ")\n";
                glfwSetWindowShouldClose(window, GLFW_TRUE);
            }
            if (now-lastTitle >= 0.25 || frames == 1) {
                char title[320];
                const char* mode = task == 1 ? (app.camera.orbitTest ? "lookAt origin" : "fixed front") :
                    task == 2 ? (app.camera.worldUp ? "world Y" : "camUp") :
                    (app.camera.speedBeforeDt ? "x2 before dt" : "x2 after dt");
                std::snprintf(title, sizeof(title), "Week 09 task %d | %s | pos %.2f %.2f %.2f | %.0f FPS | VSync %s",
                    task, mode, app.camera.position.x, app.camera.position.y, app.camera.position.z,
                    now > lastTitle ? titleFrames/(now-lastTitle) : 0, app.vsync ? "ON" : "OFF");
                glfwSetWindowTitle(window, title);
                titleFrames = 0; lastTitle = now;
            }
            glfwSwapBuffers(window);
        }
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n'; result = 1;
    }
    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);
    glDeleteProgram(program);
    glfwDestroyWindow(window);
    glfwTerminate();
    return result;
}
