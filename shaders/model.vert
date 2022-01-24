#version 450

layout(location = 0) in vec4 position;
layout(location = 1) in vec4 color;
layout(location = 2) in vec4 normal;
layout(location = 3) in vec2 uv;

layout(location = 0) out vec4 outColor;

layout(binding = 0) uniform UniformBufferObj {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;


void main() {
    gl_Position = ubo.proj * ubo.view * ubo.model * position;
    outColor = color;
}