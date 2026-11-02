#include "camera.hpp"

#include <cmath>
#include <iostream>
#include <stdexcept>

namespace {
void check(bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message);
}
void near(glm::vec3 actual, glm::vec3 expected, const char* message) {
    check(glm::length(actual - expected) < 0.0001f, message);
}
void finite(const glm::mat4& matrix) {
    for (int col = 0; col < 4; ++col)
        for (int row = 0; row < 4; ++row)
            check(std::isfinite(matrix[col][row]), "Non-finite view matrix");
}
}

int main() {
    try {
        using namespace week09;
        Input input;
        input.forward = true;
        for (int fps : {30, 60, 144, 1000}) {
            Camera camera;
            for (int frame = 0; frame < fps; ++frame)
                camera.move(input, 1.0f / fps, 1);
            near(camera.position, {0, 0, 0.5f}, "Movement depends on FPS");
        }
        Camera camera;
        input = {}; input.backward = true;
        camera.move(input, 1, 1);
        near(camera.position, {0, 0, 5.5f}, "S direction");
        camera = {}; input = {}; input.left = true;
        camera.move(input, 1, 1);
        near(camera.position, {-2.5f, 0, 3}, "A direction");
        input = {}; input.right = true;
        camera.move(input, 1, 1);
        near(camera.position, {0, 0, 3}, "D direction");
        input = {}; input.up = true;
        camera.move(input, 1, 2);
        near(camera.position, {0, 2.5f, 3}, "Space direction");
        input = {}; input.down = true;
        camera.move(input, 2, 2);
        near(camera.position, {0, -2.5f, 3}, "Shift direction");
        for (bool before : {false, true}) {
            camera = {}; camera.speedBeforeDt = before;
            input = {}; input.forward = input.sprint = true;
            camera.move(input, 1, 3);
            near(camera.position, {0, 0, -2}, "Ctrl should double speed");
            camera = {}; camera.move(input, 1, 1);
            near(camera.position, {0, 0, 0.5f}, "Sprint leaked into task 1");
        }
        camera = {}; input = {}; input.up = true;
        camera.up = glm::normalize(glm::vec3(0, 1, 1));
        camera.worldUp = false;
        camera.move(input, 1, 2);
        near(camera.position, glm::vec3(0, 0, 3) + 2.5f * camera.up, "camUp mode");
        camera.position = {0, 0, 3}; camera.worldUp = true;
        camera.move(input, 1, 2);
        near(camera.position, {0, 2.5f, 3}, "World Y mode");
        camera = {}; camera.orbitTest = true; camera.position = {2.5f, 0, 3};
        const glm::vec4 target = camera.view() * glm::vec4(0, 0, 0, 1);
        check(std::abs(target.x) < 0.0001f && std::abs(target.y) < 0.0001f && target.z < 0,
              "Origin should stay in the centre in lookAt test");
        camera.position = {0, 0, 0}; finite(camera.view());
        camera.position = {0, 3, 0}; finite(camera.view());
        std::cout << "PASS: WASD, vertical modes, sprint, 30/60/144/1000 FPS, lookAt singularities\n";
    } catch (const std::exception& error) {
        std::cerr << "FAIL: " << error.what() << '\n';
        return 1;
    }
}
