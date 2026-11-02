#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace week09 {

struct Input {
    bool forward = false, backward = false, left = false, right = false;
    bool up = false, down = false, sprint = false;
};

struct Camera {
    glm::vec3 position{0.0f, 0.0f, 3.0f};
    glm::vec3 front{0.0f, 0.0f, -1.0f};
    glm::vec3 up{0.0f, 1.0f, 0.0f};
    bool orbitTest = false;
    bool worldUp = true;
    bool speedBeforeDt = false;

    void move(const Input& input, float dt, int task) {
        // В задании 3 сравниваются оба порядка умножения на dt и на 2.
        float speed = 2.5f;
        if (speedBeforeDt && task == 3 && input.sprint) speed *= 2.0f;
        speed *= dt;
        if (!speedBeforeDt && task == 3 && input.sprint) speed *= 2.0f;

        const glm::vec3 right = glm::normalize(glm::cross(front, up));
        if (input.forward) position += speed * front;
        if (input.backward) position -= speed * front;
        if (input.left) position -= speed * right;
        if (input.right) position += speed * right;
        if (task == 2) {
            const glm::vec3 vertical = worldUp ? glm::vec3(0, 1, 0) : up;
            if (input.up) position += speed * vertical;
            if (input.down) position -= speed * vertical;
        }
    }

    glm::mat4 view() const {
        glm::vec3 direction = orbitTest ? -position : front;
        // eye == target не задаёт направление: используем camFront.
        if (glm::dot(direction, direction) < 1e-10f) direction = front;
        direction = glm::normalize(direction);
        glm::vec3 viewUp = up;
        // На оси Y направление на начало координат параллельно camUp.
        if (glm::dot(glm::cross(direction, viewUp),
                     glm::cross(direction, viewUp)) < 1e-10f) {
            viewUp = glm::vec3(0, 0, 1);
        }
        return glm::lookAt(position, position + direction, viewUp);
    }
};

} // namespace week09
