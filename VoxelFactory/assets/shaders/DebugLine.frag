#version 460 core
out vec4 o_FragColor;

in vec3 Color;

void main() {
    o_FragColor = vec4(Color.r, Color.g, Color.b, 1.0f);
} 