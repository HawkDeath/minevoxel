#version 450
layout (location = 0) in vec4 color;
layout (location = 1) in vec2 uv;

layout (location = 0) out vec4 FragColor;

//layout(binding = 1) uniform sampler2D textureSampler;

void main() {
    FragColor = color;
    // FragColor = texture(textureSampler, uv);
}