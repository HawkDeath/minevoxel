#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec2 uv;

layout(location = 0) out vec4 outColor;
layout(location = 1) out vec2 outTexCoord;

layout(binding = 0) uniform UniformBufferObj {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;


void main() {
    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(position, 1.0f);
    outColor = vec4(color, 1.0f);
    outTexCoord = uv;
}