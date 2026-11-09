#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <cmath>

namespace week10 {
struct Input {
    bool forward=false, backward=false, left=false, right=false;
    bool up=false, down=false, sprint=false;
};
struct Camera {
    glm::vec3 position{0,0,3}, front{0,0,-1}, up{0,1,0};
    float yaw=-90, pitch=0, fov=45;
    double lastX=400, lastY=300;
    bool firstMouse=true;
    void mouse(double x, double y) {
        if (firstMouse) { lastX=x; lastY=y; firstMouse=false; return; }
        // Mouse reports displacement, so no delta time here.
        yaw += static_cast<float>(x-lastX)*0.1f;
        pitch = std::clamp(pitch+static_cast<float>(lastY-y)*0.1f, -89.0f, 89.0f);
        lastX=x; lastY=y;
        front=glm::normalize(glm::vec3(
            std::cos(glm::radians(yaw))*std::cos(glm::radians(pitch)),
            std::sin(glm::radians(pitch)),
            std::sin(glm::radians(yaw))*std::cos(glm::radians(pitch))));
    }
    void scroll(double offset) { fov=std::clamp(fov-static_cast<float>(offset),1.0f,45.0f); }
    void move(const Input& input, float dt, int) {
        const float speed=2.5f*dt*(input.sprint ? 2.0f : 1.0f);
        const glm::vec3 right=glm::normalize(glm::cross(front,up));
        if(input.forward) position+=speed*front;
        if(input.backward) position-=speed*front;
        if(input.right) position+=speed*right;
        if(input.left) position-=speed*right;
        if(input.up) position+=speed*up;
        if(input.down) position-=speed*up;
    }
    glm::mat4 view() const { return glm::lookAt(position,position+front,up); }
};
}
