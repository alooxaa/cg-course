#version 330 core

layout (location = 0) in vec2 inPosition;
layout (location = 1) in vec3 inColor;

uniform vec2 uShift;       // куда сдвинуть фигуру (в NDC)
uniform float uAspectFix;  // высота / ширина окна, чтобы квадрат не растягивался

out vec3 vertexColor;

void main() {
    vec2 p = inPosition + uShift;
    gl_Position = vec4(p.x * uAspectFix, p.y, 0.0, 1.0);
    vertexColor = inColor;
}
